#include "Log.h"
#include "MarchingSquares.h"
#include "biome/Biome.h"
#include "entity/ClientPlayer.h"
#include "entity/Entity.h"
#include "entity/ServerPlayer.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "game/InteractionSet.h"
#include "game/ServerGame.h"
#include "graphics/RendererContext.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/TextRenderer.h"
#include "graphics/Tileset.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "packet/InteractPacket.h"
#include "packet/PlaySoundPacket.h"
#include "realm/Realm.h"
#include "realm/RealmFactory.h"
#include "threading/ThreadContext.h"
#include "tile/Tile.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "util/Cast.h"
#include "util/Timer.h"
#include "util/Util.h"

#include <iostream>
#include <thread>
#include <unordered_set>

// #define PROFILE_TICKS

namespace Game3 {
	void from_json(const nlohmann::json &json, RealmDetails &details) {
		details.tilesetName = json.at("tileset");
	}

	Realm::Realm(const GamePtr &game): weakGame(game) {
		if (game->getSide() == Side::Client) {
			game->toClient().getWindow().queue([this] {
				createRenderers();
				initRendererRealms();
				initRendererTileProviders();
				renderersReady = true;
				queueStaticLightingTexture();
			});
		}
	}

	Realm::Realm(const GamePtr &game, RealmID id_, RealmType type_, Identifier tileset_id, int64_t seed_):
	id(id_), type(std::move(type_)), tileProvider(std::move(tileset_id)), seed(seed_), weakGame(game) {
		if (game->getSide() == Side::Client) {
			game->toClient().getWindow().queue([this] {
				createRenderers();
				initRendererRealms();
				initTexture();
				initRendererTileProviders();
				renderersReady = true;
				queueStaticLightingTexture();
			});
		}
	}

	void Realm::initRendererRealms() {
		if (getSide() != Side::Client)
			return;

		assert(baseRenderers);
		assert(upperRenderers);

		for (auto &row: *baseRenderers)
			for (auto &renderer: row)
				renderer.setRealm(*this);

		for (auto &row: *upperRenderers)
			for (auto &renderer: row)
				renderer.setRealm(*this);
	}

	void Realm::initRendererTileProviders() {
		if (getSide() != Side::Client)
			return;

		assert(baseRenderers);
		assert(upperRenderers);

		for (auto &row: *baseRenderers)
			for (auto &renderer: row)
				renderer.setup(tileProvider);

		for (auto &row: *upperRenderers)
			for (auto &renderer: row)
				renderer.setup(tileProvider);
	}

	void Realm::initTexture() {}

	RealmPtr Realm::fromJSON(const GamePtr &game, const nlohmann::json &json, bool full_data) {
		const RealmType type = json.at("type");
		auto factory = game->registry<RealmFactoryRegistry>().at(type);
		assert(factory);
		auto out = (*factory)(game);
		out->absorbJSON(json, full_data);
		return out;
	}

	std::string Realm::getSQL() {
		return R"(
			CREATE TABLE IF NOT EXISTS realms (
				realmID INT PRIMARY KEY,
				json MEDIUMTEXT,
				tilesetHash VARCHAR(128)
			);
		)";
	}

	void Realm::absorbJSON(const nlohmann::json &json, bool full_data) {
		auto shared = shared_from_this();
		id = json.at("id");
		type = json.at("type");
		seed = json.at("seed");
		generatedChunks = json.at("generatedChunks");
		outdoors = json.at("outdoors");
		tileProvider.clear();

		if (json.contains("extra"))
			extraData = json.at("extra");

		initRendererTileProviders();
		initTexture();

		tileProvider.absorbJSON(json.at("provider"), full_data);

		if (full_data) {
			GamePtr game = getGame();

			{
				auto tile_entities_lock = tileEntities.uniqueLock();
				auto by_gid_lock = tileEntitiesByGID.uniqueLock();
				for (const auto &[position_string, tile_entity_json]: json.at("tileEntities").get<std::unordered_map<std::string, nlohmann::json>>()) {
					auto tile_entity = TileEntity::fromJSON(game, tile_entity_json);
					tileEntities.emplace(Position(position_string), tile_entity);
					tileEntitiesByGID[tile_entity->globalID] = tile_entity;
					attach(tile_entity);
					tile_entity->setRealm(shared);
					tile_entity->onSpawn();
				}
			}

			{
				auto entities_lock = entities.uniqueLock();
				auto by_gid_lock = entitiesByGID.uniqueLock();
				entities.clear();
				for (const auto &entity_json: json.at("entities")) {
					auto entity = *entities.insert(Entity::fromJSON(game, entity_json)).first;
					entity->setRealm(shared);
					entitiesByGID[entity->globalID] = entity;
					attach(entity);
				}
			}
		}
	}

	RealmID Realm::getID() const {
		return id;
	}

	void Realm::onFocus() {
		if (getSide() == Side::Client && !focused.exchange(true)) {
			queueStaticLightingTexture();
			wakeupPending = true;
		}
	}

	void Realm::onBlur() {
		if (getSide() == Side::Client && focused.exchange(false))
			snoozePending = true;
	}

	void Realm::onRemove() {}

	void Realm::createRenderers() {
		if (getSide() != Side::Client)
			return;

		getGame()->toClient().activateContext();
		baseRenderers.emplace();
		upperRenderers.emplace();
	}

	bool Realm::prerender() {
		if (getSide() != Side::Client)
			return false;

		if (!focused)
			onFocus();

		ClientGame &game = getGame()->toClient();
		PlayerPtr player = game.getPlayer();
		assert(player);

		const ChunkPosition current_chunk = player->getChunk();
		if (staticLightingQueued.exchange(false) || lastPlayerChunk != current_chunk) {
			lastPlayerChunk = current_chunk;
			remakeStaticLightingTexture();
			return true;
		}

		return false;
	}

	void Realm::render(const int width, const int height, const std::pair<double, double> &center, float scale, RendererContext &renderers, float game_time) {
		if (getSide() != Side::Client)
			return;

		if (!focused)
			onFocus();

		ClientGame &game = getGame()->toClient();
		PlayerPtr player = game.getPlayer();
		assert(player);

		auto &[rectangle_renderer, single_sprite, batch_sprite, text_renderer, circle_renderer, recolor, settings, factor] = renderers;

		float effective_time = outdoors? game_time : 1;

		if (baseRenderers) {
			{
				auto lock = settings.sharedLock();
				if (!settings.renderLighting)
					effective_time = 1;
			}

			for (auto &row: *baseRenderers) {
				for (ElementBufferedRenderer &renderer: row) {
					renderer.onBackbufferResized(width, height);
					renderer.render(effective_time, scale, center.first, center.second); CHECKGL
				}
			}
		}

		batch_sprite.centerX = center.first;
		batch_sprite.centerY = center.second;
		// batch_sprite.divisor = outdoors? game_time : 1;
		text_renderer.centerX = center.first;
		text_renderer.centerY = center.second;

		{
			auto lock = tileEntities.sharedLock();
			for (const auto &[index, tile_entity]: tileEntities) {
				tile_entity->render(batch_sprite); CHECKGL
			}
		}

		batch_sprite.renderNow();

		std::set<EntityPtr, EntityZCompare> rendered_entities;

		ChunkRange(player->getChunk()).iterate([&](ChunkPosition chunk_position) {
			if (auto entities_in_chunk = getEntities(chunk_position)) {
				auto lock = entities_in_chunk->sharedLock();
				for (const WeakEntityPtr &weak_entity: *entities_in_chunk) {
					EntityPtr entity = weak_entity.lock();
					if (entity && entity != player) {
						rendered_entities.insert(entity);
						entity->render(renderers); CHECKGL
					}
				}
			}
		});

		player->render(renderers);

		batch_sprite.renderNow();

		if (upperRenderers) {
			for (auto &row: *upperRenderers) {
				for (UpperRenderer &renderer: row) {
					renderer.onBackbufferResized(width, height);
					renderer.render(effective_time, scale, center.first, center.second); CHECKGL
				}
			}
		}

		{
			auto lock = tileEntities.sharedLock();
			for (const auto &[index, tile_entity]: tileEntities) {
				tile_entity->renderUpper(batch_sprite); CHECKGL
			}
		}

		for (const EntityPtr &entity: rendered_entities) {
			entity->renderUpper(renderers); CHECKGL
		}

		player->renderUpper(renderers);
		batch_sprite.renderNow();
	}

	void Realm::renderLighting(const int, const int, const std::pair<double, double> &, float, RendererContext &renderers, float game_time) {
		if (getSide() != Side::Client)
			return;

		clearLighting(game_time);

		ClientGame &game = getGame()->toClient();
		auto player = game.getPlayer();
		assert(player);

		{
			auto lock = tileEntities.sharedLock();
			for (const auto &[index, tile_entity]: tileEntities)
				tile_entity->renderLighting(renderers);
		}

		ChunkRange(player->getChunk()).iterate([&](ChunkPosition chunk_position) {
			if (auto entities_in_chunk = getEntities(chunk_position)) {
				auto lock = entities_in_chunk->sharedLock();
				for (const WeakEntityPtr &weak_entity: *entities_in_chunk) {
					EntityPtr entity = weak_entity.lock();
					if (entity && entity != player)
						entity->renderLighting(renderers);
				}
			}
		});

		player->renderLighting(renderers);

		renderers.batchSprite.renderNow();
	}

	void Realm::clearLighting(float) {
		Color color{1, 1, 1, 1};

		if (outdoors) {
			const double hour = getGame()->getHour();
			if (hour <= 4)
				color = Color(0x2a3273ff);
			else if (hour < 4.375)
				color = lerp(Color(0x2a3273ff), Color(0x863e7eff), (hour - 4.0) / 0.375);
			else if (hour < 4.75)
				color = lerp(Color(0x863e7eff), Color(0xca6262ff), (hour - 4.375) / 0.375);
			else if (hour < 5.125)
				color = lerp(Color(0xca6262ff), Color(0xebb59eff), (hour - 4.75) / 0.375);
			else if (hour < 5.5)
				color = lerp(Color(0xebb59eff), Color(0xffffffff), (hour - 5.125) / 0.375);
			else if (hour <= 19.5)
				color = Color(0xffffffff);
			else if (hour < 19.875)
				color = lerp(Color(0xffffffff), Color(0xebb59eff), (hour - 19.5) / 0.375);
			else if (hour < 20.25)
				color = lerp(Color(0xebb59eff), Color(0xca6262ff), (hour - 19.875) / 0.375);
			else if (hour < 20.625)
				color = lerp(Color(0xca6262ff), Color(0x863e7eff), (hour - 20.25) / 0.375);
			else if (hour < 21)
				color = lerp(Color(0x863e7eff), Color(0x2a3273ff), (hour - 20.625) / 0.375);
			else
				color = Color(0x2a3273ff);
		}

		glClearColor(color.red, color.green, color.blue, color.alpha); CHECKGL
		glClear(GL_COLOR_BUFFER_BIT); CHECKGL
	}

	void Realm::reupload() {
		if (getSide() != Side::Client)
			return;

		getGame()->toClient().activateContext();

		for (auto &row: *baseRenderers)
			for (auto &renderer: row)
				renderer.reupload();

		for (auto &row: *upperRenderers)
			for (auto &renderer: row)
				renderer.reupload();
	}

	EntityPtr Realm::add(const EntityPtr &entity, const Position &position) {
		if (auto found = getEntity(entity->getGID()))
			return found;
		auto shared = shared_from_this();
		{
			auto lock = entities.uniqueLock();
			entities.insert(entity);
		}
		{
			auto lock = entitiesByGID.uniqueLock();
			entitiesByGID[entity->globalID] = entity;
		}
		entity->firstTeleport = true;
		if (entity->isPlayer() && entity->weakRealm.lock())
			safeDynamicCast<Player>(entity)->stopMoving();
		entity->setRealm(shared);
		entity->teleport(position, MovementContext{.excludePlayer = entity->getGID(), .clearOffset = false, .isTeleport = true});
		entity->firstTeleport = false;
		attach(entity);
		if (entity->isPlayer()) {
			{
				auto lock = players.uniqueLock();
				players.emplace(safeDynamicCast<Player>(entity));
			}
			recalculateVisibleChunks();
		}
		return entity;
	}

	TileEntityPtr Realm::add(const TileEntityPtr &tile_entity) {
		{
			auto lock = tileEntities.sharedLock();
			if (tileEntities.contains(tile_entity->position))
				return nullptr;
		}
		tile_entity->setRealm(shared_from_this());
		if (!tile_entity->initialized) {
			GamePtr game = getGame();
			tile_entity->init(*game);
		}
		{
			auto lock = tileEntities.uniqueLock();
			tileEntities.emplace(tile_entity->position, tile_entity);
		}
		{
			auto lock = tileEntitiesByGID.uniqueLock();
			tileEntitiesByGID[tile_entity->globalID] = tile_entity;
		}
		attach(tile_entity);
		if (tile_entity->solid) {
			std::unique_lock<std::shared_mutex> path_lock;
			tileProvider.findPathState(tile_entity->position.copyBase(), &path_lock) = 0;
		}
		tile_entity->onSpawn();
		return tile_entity;
	}

	void Realm::initEntities() {
		auto entities_lock = entities.sharedLock();
		for (const auto &entity: entities) {
			entity->setRealm(shared_from_this());
			if (auto player = std::dynamic_pointer_cast<Player>(entity)) {
				auto players_lock = players.uniqueLock();
				players.insert(player);
			}
		}
	}

	void Realm::tick(float delta) {
		if (ticking.exchange(true))
			return;

#ifdef PROFILE_TICKS
		Timer realm_timer{"TickRealm"};
#endif

		for (const auto &[entity, position]: entityInitializationQueue.steal())
			initEntity(entity, position);

		for (const auto &[entity, position]: entityAdditionQueue.steal())
			add(entity, position);

		for (const auto &stolen: tileEntityAdditionQueue.steal())
			if (auto locked = stolen.lock())
				add(locked);

		GamePtr game = getGame();
		const TickArgs args{game, game->getCurrentTick(), delta};

		if (isServer()) {
			std::vector<RemoteClient::BufferGuard> guards;

			{
				auto lock = players.sharedLock();
				guards.reserve(players.size());
				for (const auto &weak_player: players) {
					if (auto player = weak_player.lock()) {
						if (auto client = player->toServer()->weakClient.lock())
							guards.emplace_back(client);

						if (!player->ticked && player->tryInitialTick()) {
							player->ticked = true;
							player->tick(args);
						}
					}
				}
			}

			{
				auto lock = villages.sharedLock();
				for (const VillagePtr &village: villages) {
					if (village->tryInitialTick())
						village->tick(args);
				}
			}

			{
				auto visible_lock = visibleChunks.sharedLock();
				for (const auto &chunk: visibleChunks) {
					{
						auto by_chunk_lock = entitiesByChunk.sharedLock();
						if (auto iter = entitiesByChunk.find(chunk); iter != entitiesByChunk.end() && iter->second) {
							auto set_lock = iter->second->sharedLock();
							by_chunk_lock.unlock();
							for (const WeakEntityPtr &weak_entity: *iter->second) {
								EntityPtr entity = weak_entity.lock();
								if (entity && !entity->isPlayer() && entity->tryInitialTick()) {
#ifdef PROFILE_TICKS
									Timer timer{"TickEntity"};
#endif
									entity->tick(args);
								}
							}
						}
					}
					{
						auto by_chunk_lock = tileEntitiesByChunk.sharedLock();
						if (auto iter = tileEntitiesByChunk.find(chunk); iter != tileEntitiesByChunk.end() && iter->second) {
							auto set = iter->second;
							auto set_lock = set->sharedLock();
							by_chunk_lock.unlock();
							for (const auto &tile_entity: *set) {
#ifdef PROFILE_TICKS
								Timer timer{"TickTileEntity"};
#endif
								if (tile_entity->tryInitialTick()) {
									tile_entity->tick(args);
								}
							}
						}
					}

					std::uniform_int_distribution<int64_t> distribution{0, CHUNK_SIZE - 1};
					Tileset &tileset = getTileset();
					auto shared = shared_from_this();

#ifdef PROFILE_TICKS
					Timer timer{"RandomTicks"};
#endif
					for (size_t i = 0; i < game->randomTicksPerChunk; ++i) {
						const Position position(chunk.y * CHUNK_SIZE + distribution(threadContext.rng), chunk.x * CHUNK_SIZE + distribution(threadContext.rng));

						for (const Layer layer: mainLayers)
							if (auto tile_id = tileProvider.tryTile(layer, position); tile_id && *tile_id != 0)
								game->getTile(tileset[*tile_id])->randomTick({position, shared, nullptr});
					}
				}
			}

			for (const auto &stolen: entityRemovalQueue.steal())
				if (auto locked = stolen.lock())
					remove(locked);

			for (const auto &stolen: entityDestructionQueue.steal())
				if (auto locked = stolen.lock())
					locked->destroy();

			for (const auto &stolen: tileEntityRemovalQueue.steal())
				if (auto locked = stolen.lock())
					remove(locked);

			for (const auto &stolen: tileEntityDestructionQueue.steal())
				if (auto locked = stolen.lock())
					locked->destroy();

			for (const auto &stolen: playerRemovalQueue.steal())
				if (auto locked = stolen.lock())
					removePlayer(locked);

			for (const auto &stolen: generalQueue.steal())
				stolen();

			if (!tileProvider.generationQueue.empty()) {
				const auto chunk_position = tileProvider.generationQueue.take();
				if (!generatedChunks.contains(chunk_position)) {
					tileProvider.ensureAllChunks(chunk_position);
					generateChunk(chunk_position);
					generatedChunks.insert(chunk_position);
					remakePathMap(chunk_position);
					auto lock = chunkRequests.uniqueLock();
					if (auto iter = chunkRequests.find(chunk_position); iter != chunkRequests.end()) {
						std::unordered_set<std::shared_ptr<RemoteClient>> strong;
						for (const auto &weak: iter->second)
							if (auto locked = weak.lock())
								strong.insert(locked);
						sendToMany(strong, chunk_position);
						chunkRequests.erase(iter);
					}
				}
			} else {
				auto lock = chunkRequests.uniqueLock();

				if (!chunkRequests.empty()) {
					auto iter = chunkRequests.begin();
					const auto &[chunk_position, client_set] = *iter;

					if (!generatedChunks.contains(chunk_position)) {
						generateChunk(chunk_position);
						generatedChunks.insert(chunk_position);
						remakePathMap(chunk_position);
					}

					sendToMany(filterWeak(client_set), chunk_position);
					chunkRequests.erase(iter);
				}
			}

		} else {

			auto player = getGame()->toClient().getPlayer();
			if (!player)
				return;

			const auto player_cpos = player->getPosition().getChunk();

			{
				auto lock = entities.sharedLock();
				for (const auto &entity: entities) {
					if (entity->tryInitialTick()) {
						entity->tick(args);
					}
				}
			}

			{
				auto lock = tileEntities.sharedLock();
				for (auto &[index, tile_entity]: tileEntities) {
					if (tile_entity->tryInitialTick()) {
						tile_entity->tick(args);
					}
				}
			}

			ticking = false;

			for (const auto &stolen: entityRemovalQueue.steal())
				if (auto locked = stolen.lock())
					removeSafe(locked);

			for (const auto &stolen: entityDestructionQueue.steal())
				if (auto locked = stolen.lock())
					locked->destroy();

			for (const auto &stolen: tileEntityRemovalQueue.steal())
				if (auto locked = stolen.lock())
					removeSafe(locked);

			for (const auto &stolen: tileEntityDestructionQueue.steal())
				if (auto locked = stolen.lock())
					locked->destroy();

			for (const auto &stolen: generalQueue.steal())
				stolen();

			if (renderersReady) {
				Index row_index = 0;

				for (auto &row: *baseRenderers) {
					Index col_index = 0;
					for (auto &renderer: row) {
						renderer.setChunkPosition({
							static_cast<int32_t>(player_cpos.x + col_index - REALM_DIAMETER / 2 - 1),
							static_cast<int32_t>(player_cpos.y + row_index - REALM_DIAMETER / 2 - 1),
						});
						++col_index;
					}
					++row_index;
				}

				row_index = 0;
				for (auto &row: *upperRenderers) {
					Index col_index = 0;
					for (auto &renderer: row) {
						renderer.setChunkPosition({
							static_cast<int32_t>(player_cpos.x + col_index - REALM_DIAMETER / 2 - 1),
							static_cast<int32_t>(player_cpos.y + row_index - REALM_DIAMETER / 2 - 1),
						});
						++col_index;
					}
					++row_index;
				}
			}
		}

		ticking = false;
	}

	std::vector<EntityPtr> Realm::findEntities(const Position &position) const {
		WeakEntitySet entity_set = getEntities(position.getChunk());
		if (!entity_set)
			return {};

		std::vector<EntityPtr> out;
		auto lock = entity_set->sharedLock();

		for (const WeakEntityPtr &weak_entity: *entity_set) {
			EntityPtr entity = weak_entity.lock();
			if (entity && entity->occupies(position))
				out.push_back(entity);
		}

		return out;
	}

	bool Realm::hasEntities(const Position &position) const {
		WeakEntitySet entity_set = getEntities(position.getChunk());
		if (!entity_set)
			return {};

		auto lock = entity_set->sharedLock();

		for (const WeakEntityPtr &weak_entity: *entity_set) {
			EntityPtr entity = weak_entity.lock();
			if (entity && entity->occupies(position))
				return true;
		}

		return false;
	}

	bool Realm::hasEntities(const Position &position, const std::function<bool(const EntityPtr &)> &predicate) const {
		WeakEntitySet entity_set = getEntities(position.getChunk());
		if (!entity_set)
			return {};

		auto lock = entity_set->sharedLock();

		for (const WeakEntityPtr &weak_entity: *entity_set) {
			EntityPtr entity = weak_entity.lock();
			if (entity && entity->occupies(position) && predicate(entity))
				return true;
		}

		return false;
	}

	size_t Realm::countEntities(const Position &position) const {
		WeakEntitySet entity_set = getEntities(position.getChunk());
		if (!entity_set)
			return {};

		auto lock = entity_set->sharedLock();
		size_t out = 0;

		for (const WeakEntityPtr &weak_entity: *entity_set) {
			EntityPtr entity = weak_entity.lock();
			if (entity && entity->occupies(position))
				++out;
		}

		return out;
	}

	size_t Realm::countEntities(const Position &position, const std::function<bool(const EntityPtr &)> &predicate) const {
		WeakEntitySet entity_set = getEntities(position.getChunk());
		if (!entity_set)
			return {};

		auto lock = entity_set->sharedLock();
		size_t out = 0;

		for (const WeakEntityPtr &weak_entity: *entity_set) {
			EntityPtr entity = weak_entity.lock();
			if (entity && entity->occupies(position) && predicate(entity))
				++out;
		}

		return out;
	}

	std::vector<EntityPtr> Realm::findEntitiesSquare(const Position &position, uint64_t radius) const {
		if (radius == 1)
			return findEntities(position);

		if (radius == 0)
			return {};

		std::vector<EntityPtr> out;

		const Position offset(radius - 1, radius - 1);
		ChunkRange((position - offset).getChunk(), (position + offset).getChunk()).iterate([this, &out, position, radius](ChunkPosition chunk_position) {
			WeakEntitySet entity_set = getEntities(chunk_position);
			if (!entity_set)
				return;

			auto lock = entity_set->sharedLock();
			for (const WeakEntityPtr &weak_entity: *entity_set) {
				EntityPtr entity = weak_entity.lock();
				if (entity && entity->position.copyBase().maximumAxisDistance(position) < radius)
					out.push_back(entity);
			}
		});

		return out;
	}

	std::vector<EntityPtr> Realm::findEntitiesSquare(const Position &position, uint64_t radius, const std::function<bool(const EntityPtr &)> &filter) const {
		if (radius == 1)
			return findEntities(position);

		if (radius == 0)
			return {};

		std::vector<EntityPtr> out;

		const Position offset(radius - 1, radius - 1);
		ChunkRange((position - offset).getChunk(), (position + offset).getChunk()).iterate([this, &out, &filter, position, radius](ChunkPosition chunk_position) {
			WeakEntitySet entity_set = getEntities(chunk_position);
			if (!entity_set)
				return;

			auto lock = entity_set->sharedLock();
			for (const WeakEntityPtr &weak_entity: *entity_set) {
				EntityPtr entity = weak_entity.lock();
				if (entity && entity->position.copyBase().maximumAxisDistance(position) < radius && filter(entity))
					out.push_back(entity);
			}
		});

		return out;
	}

	bool Realm::hasEntitiesSquare(const Position &position, uint64_t radius, const std::function<bool(const EntityPtr &)> &predicate) const {
		if (radius == 0)
			return false;

		bool out = false;

		const Position offset(radius - 1, radius - 1);
		ChunkRange((position - offset).getChunk(), (position + offset).getChunk()).iterate([this, &out, &predicate, position, radius](ChunkPosition chunk_position) {
			WeakEntitySet entity_set = getEntities(chunk_position);
			if (!entity_set)
				return false;

			auto lock = entity_set->sharedLock();
			for (const WeakEntityPtr &weak_entity: *entity_set) {
				EntityPtr entity = weak_entity.lock();
				if (entity && entity->position.copyBase().maximumAxisDistance(position) < radius && predicate(entity)) {
					out = true;
					return true;
				}
			}

			return false;
		});

		return out;
	}

	std::vector<EntityPtr> Realm::findEntities(const Position &position, const EntityPtr &except) {
		std::vector<EntityPtr> out;
		auto lock = entities.sharedLock();
		for (const EntityPtr &entity: entities)
			if (entity->occupies(position) && entity != except)
				out.push_back(entity);
		return out;
	}

	EntityPtr Realm::findEntity(const Position &position, const EntityPtr &except, bool single_chunk) {
		if (!single_chunk) {
			auto lock = entities.sharedLock();
			for (const EntityPtr &entity: entities)
				if (entity->occupies(position) && entity != except)
					return entity;
			return {};
		}

		auto by_chunk_lock = entitiesByChunk.sharedLock();
		auto iter = entitiesByChunk.find(position.getChunk());
		if (iter == entitiesByChunk.end())
			return {};
		auto found_entities = iter->second;
		if (!found_entities)
			return {};
		by_chunk_lock.unlock();

		auto found_lock = found_entities->sharedLock();
		for (const WeakEntityPtr &weak_entity: *found_entities) {
			EntityPtr entity = weak_entity.lock();
			if (entity && entity->occupies(position) && entity != except)
				return entity;
		}
		return {};
	}

	TileEntityPtr Realm::tileEntityAt(Position position) {
		auto lock = tileEntities.sharedLock();
		if (auto iter = tileEntities.find(position); iter != tileEntities.end())
			return iter->second;
		return {};
	}

	void Realm::remove(const EntityPtr &entity) {
		entitiesByGID.erase(entity->globalID);
		detach(entity);
		if (auto player = std::dynamic_pointer_cast<Player>(entity))
			removePlayer(player);
		entities.erase(entity);
	}

	void Realm::removeSafe(const EntityPtr &entity) {
		auto entity_lock = entities.uniqueLock();
		auto by_gid_lock = entitiesByGID.uniqueLock();
		remove(entity);
	}

	void Realm::eviscerate(const EntityPtr &entity, bool can_warn) {
		{
			auto lock = entities.uniqueLock();
			if (auto iter = entities.find(entity); iter != entities.end()) {
				if (can_warn)
					WARN("Still present in Realm {}'s entities", id);
				entities.erase(iter);
			}
		}

		{
			auto lock = entitiesByGID.uniqueLock();
			if (auto iter = entitiesByGID.find(entity->getGID()); iter != entitiesByGID.end()) {
				if (can_warn)
					WARN("Still present in Realm {}'s entitiesByGID", id);
				entitiesByGID.erase(iter);
			}
		}

		{
			auto lock = entitiesByChunk.sharedLock();
			for (const auto &[chunk_position, set]: entitiesByChunk) {
				if (!set)
					continue;
				auto set_lock = set->uniqueLock();
				if (auto iter = set->find(entity); iter != set->end()) {
					if (can_warn)
						WARN("Still present in Realm {}'s entitiesByChunk at chunk position {}", id, chunk_position);
					set->erase(iter);
				}
			}
		}

		{
			auto lock = entityInitializationQueue.sharedLock();
			for (const auto &[to_init, position]: entityInitializationQueue.get()) {
				if (to_init == entity) {
					ERROR("{} {} inexplicably found in realm {}'s initialization queue", entity->getName(), entity->getGID(), id);
				}
			}
		}

		{
			auto lock = entityAdditionQueue.sharedLock();
			for (const auto &[to_add, position]: entityAdditionQueue.get()) {
				if (to_add == entity) {
					ERROR("{} {} inexplicably found in realm {}'s addition queue", entity->getName(), entity->getGID(), id);
				}
			}
		}
	}

	void Realm::remove(const TileEntityPtr &tile_entity, bool run_helper) {
		const Position position = tile_entity->getPosition();
		auto iter = tileEntities.find(position);
		if (iter == tileEntities.end()) {
			WARN("Can't remove tile entity: not found");
			return; // Probably already destroyed. Could happen if the tile entity was queued for removal multiple times in the same tick.
		}
		iter->second->onRemove();
		tileEntities.erase(iter);
		tileEntitiesByGID.erase(tile_entity->globalID);
		detach(tile_entity);

		if (const auto count = tile_entity.use_count(); 3 < count)
			WARN("Tile entity use count: {}", count);

		if (run_helper) {
			setLayerHelper(position.row, position.column, Layer::Submerged);
			setLayerHelper(position.row, position.column, Layer::Objects);
			queueStaticLightingTexture();
		}

		updateNeighbors(position, Layer::Submerged);
		updateNeighbors(position, Layer::Objects);
	}

	void Realm::removeSafe(const TileEntityPtr &tile_entity) {
		auto lock = tileEntities.uniqueLock();
		remove(tile_entity, true);
	}

	void Realm::onMoved(const EntityPtr &entity, const Position &old_position, const Vector3 &old_offset, const Position &new_position, const Vector3 &new_offset) {
		if (old_position != new_position) {
			if (TileEntityPtr tile_entity = tileEntityAt(old_position))
				tile_entity->onOverlapEnd(entity);

			if (TileEntityPtr tile_entity = tileEntityAt(new_position))
				tile_entity->onOverlap(entity);
		} else if (TileEntityPtr tile_entity = tileEntityAt(new_position)) {
			const bool old_grounded = old_offset.isGrounded();
			const bool new_grounded = new_offset.isGrounded();

			if (old_grounded != new_grounded) {
				if (old_grounded)
					tile_entity->onOverlapEnd(entity);
				else
					tile_entity->onOverlap(entity);
			}
		}
	}

	GamePtr Realm::getGame() const {
		return weakGame.lock();
	}

	void Realm::queueRemoval(const EntityPtr &entity) {
		entityRemovalQueue.push(entity);
	}

	void Realm::queueRemoval(const TileEntityPtr &tile_entity) {
		tileEntityRemovalQueue.push(tile_entity);
	}

	void Realm::queueDestruction(const EntityPtr &entity) {
		if (entity->isPlayer())
			INFO("Queueing player {} for entity destruction.", entity->getGID());
		entityDestructionQueue.push(entity);
	}

	void Realm::queueDestruction(const TileEntityPtr &tile_entity) {
		tileEntityDestructionQueue.push(tile_entity);
	}

	void Realm::queuePlayerRemoval(const PlayerPtr &player) {
		playerRemovalQueue.push(player);
	}

	void Realm::queueAddition(const EntityPtr &entity, const Position &new_position) {
		entityAdditionQueue.emplace(entity, new_position);
	}

	void Realm::queueAddition(const TileEntityPtr &tile_entity) {
		tileEntityAdditionQueue.push(tile_entity);
	}

	void Realm::queue(std::function<void()> fn) {
		generalQueue.push(std::move(fn));
	}

	void Realm::absorb(const EntityPtr &entity, const Position &position) {
		if (auto realm = entity->weakRealm.lock())
			realm->remove(entity);
		entity->setRealm(shared_from_this());
		GamePtr game = getGame();
		entity->init(game);
		entity->teleport(position);
	}

	void Realm::setTile(Layer layer, Index row, Index column, TileID tile_id, bool run_helper, TileUpdateContext context) {
		setTile(layer, Position(row, column), tile_id, run_helper, context);
	}

	void Realm::setTile(Layer layer, const Position &position, TileID tile_id, bool run_helper, TileUpdateContext context) {
		bool affected_lighting = false;
		GamePtr game = getGame();

		{
			std::unique_lock<std::shared_mutex> tile_lock;
			auto &tile = tileProvider.findTile(layer, position, &tile_lock, TileProvider::TileMode::Create);
			if (tile == tile_id)
				return;

			if (isClient())
				if (auto tile_object = game->getTile(getTileset()[tile]); tile_object)
					affected_lighting = tile_object->hasStaticLighting();

			tile = tile_id;
		}

		if (isServer()) {
			if (!isGenerating()) {
				tileProvider.updateChunk(position.getChunk());
				game->toServer().broadcastTileUpdate(id, layer, position, tile_id);
			}
			if (run_helper)
				setLayerHelper(position.row, position.column, layer, context);
		} else if (run_helper) {
			if (affected_lighting)
				queueStaticLightingTexture();
			else if (auto tile = game->getTile(getTileset()[tile_id]); tile && tile->hasStaticLighting())
				queueStaticLightingTexture();
		}
	}

	void Realm::setTile(Layer layer, const Position &position, const Identifier &tilename, bool run_helper, TileUpdateContext context) {
		setTile(layer, position, getTileset()[tilename], run_helper, context);
	}

	void Realm::setFluid(const Position &position, FluidTile tile) {
		{
			std::unique_lock<std::shared_mutex> fluid_lock;
			FluidTile &fluid = tileProvider.findFluid(position, &fluid_lock);
			if (fluid == tile)
				return;
			fluid = tile;
		}

		if (isServer() && !isGenerating()) {
			tileProvider.updateChunk(position.getChunk());
			getGame()->toServer().broadcastFluidUpdate(id, position, tile);
		}
	}

	void Realm::setFluid(const Position &position, const Identifier &fluidname, FluidLevel level, bool infinite) {
		std::shared_ptr<Fluid> fluid = getGame()->registry<FluidRegistry>().at(fluidname);
		assert(fluid);
		setFluid(position, FluidTile(fluid->registryID, level, infinite));
	}

	bool Realm::hasFluid(const Position &position, FluidLevel minimum) const {
		if (auto fluid = tileProvider.copyFluidTile(position))
			return minimum <= fluid->level;
		return false;
	}

	TileID Realm::getTile(Layer layer, const Position &position) const {
		return tileProvider.copyTile(layer, position, TileProvider::TileMode::Throw);
	}

	bool Realm::middleEmpty(const Position &position) const {
		const auto submerged = tryTile(Layer::Submerged, position);
		const auto object = tryTile(Layer::Objects, position);
		const auto empty = getTileset().getEmptyID();
		assert(submerged.has_value() == object.has_value());
		return (!submerged && !object) || (submerged && *submerged == empty && *object == empty);
	}

	std::optional<TileID> Realm::tryTile(Layer layer, const Position &position) const {
		return tileProvider.tryTile(layer, position);
	}

	std::optional<FluidTile> Realm::tryFluid(const Position &position) const {
		return tileProvider.copyFluidTile(position);
	}

	bool Realm::interactGround(const PlayerPtr &player, const Position &position, Modifiers modifiers, const ItemStackPtr &used_item, Hand hand) {
		const Place place(position, shared_from_this(), player);
		GamePtr game = getGame();

		if (auto iter = game->interactionSets.find(type); iter != game->interactionSets.end())
			if (iter->second->interact(place, modifiers, used_item, hand))
				return true;

		Tileset &tileset = getTileset();

		for (const Layer layer: reverse(mainLayers))
			if (std::optional<TileID> tile = tryTile(layer, position))
				if (game->getTile(tileset[*tile])->interact(place, layer, used_item, hand))
					return true;

		return false;
	}

	std::optional<Position> Realm::getPathableAdjacent(const Position &position) const {
		Position next = {position.row + 1, position.column};

		if (auto state = tileProvider.copyPathState(next); state && *state)
			return next;

		next = {position.row, position.column + 1};
		if (auto state = tileProvider.copyPathState(next); state && *state)
			return next;

		next = {position.row - 1, position.column};
		if (auto state = tileProvider.copyPathState(next); state && *state)
			return next;

		next = {position.row, position.column - 1};
		if (auto state = tileProvider.copyPathState(next); state && *state)
			return next;

		return std::nullopt;
	}

	bool Realm::isPathable(const Position &position) const {
		if (auto result = tileProvider.copyPathState(position))
			return *result;
		return false;
	}

	void Realm::setPathable(const Position &position, bool pathable) {
		std::unique_lock<std::shared_mutex> lock;
		tileProvider.findPathState(position, &lock) = pathable;
	}

	void Realm::updateNeighbors(const Position &position, Layer layer, TileUpdateContext context) {
		if (updatesPaused || context--.limit == 0)
			return;

		++threadContext.updateNeighborsDepth;

		GamePtr game = getGame();
		TilesetPtr tileset = tileProvider.getTileset(*game);
		RealmPtr self = shared_from_this();

		Place place{{}, self, nullptr};

		for (Index row_offset = -1; row_offset <= 1; ++row_offset) {
			for (Index column_offset = -1; column_offset <= 1; ++column_offset) {
				if (row_offset != 0 || column_offset != 0) {
					const Position offset_position = position + Position(row_offset, column_offset);
					if (auto neighbor = tileEntityAt(offset_position))
						neighbor->onNeighborUpdated(Position(-row_offset, -column_offset));

					if (auto tile_id = tryTile(layer, offset_position)) {
						place.position = offset_position;
						if (game->getTile((*tileset)[*tile_id])->update(place, layer))
							continue;
					}

					autotile(offset_position, layer, context);
				}
			}
		}

		if (--threadContext.updateNeighborsDepth == 0) {
			if (getSide() == Side::Client)
				queueReupload();
			threadContext.updatedLayers.clear();
		}
	}

	bool Realm::hasTileEntityAt(const Position &position) const {
		return tileEntities.contains(position);
	}

	void Realm::damageGround(const Position &position) {
		const Place place(position, shared_from_this(), nullptr);
		GamePtr game = getGame();

		if (auto iter = game->interactionSets.find(type); iter != game->interactionSets.end())
			iter->second->damageGround(place);
	}

	Tileset & Realm::getTileset() const {
		GamePtr game = getGame();
		return *tileProvider.getTileset(*game);
	}

	void Realm::toJSON(nlohmann::json &json, bool full_data) const {
		json["id"] = id;
		json["type"] = type;
		json["seed"] = seed;
		json["outdoors"] = outdoors;
		json["generatedChunks"] = generatedChunks;
		if (!extraData.empty())
			json["extra"] = extraData;

		tileProvider.toJSON(json["provider"], full_data);

		if (full_data) {
			auto &tile_entities = json["tileEntities"];
			tile_entities = std::unordered_map<std::string, nlohmann::json>{};
			for (const auto &[position, tile_entity]: tileEntities)
				tile_entities[position.simpleString()] = *tile_entity;
			json["entities"] = std::vector<nlohmann::json>();
			for (const auto &entity: entities) {
				nlohmann::json entity_json;
				entity->toJSON(entity_json);
				json["entities"].push_back(std::move(entity_json));
			}
		}
	}

	void Realm::queueEntityInit(EntityPtr entity, const Position &position) {
		entityInitializationQueue.emplace(std::move(entity), position);
	}

	void Realm::spawn(const EntityPtr &entity, const Position &position) {
		entity->spawning = true;
		entity->setRealm(shared_from_this());
		queueEntityInit(entity, position);
	}

	bool Realm::isWalkable(Index row, Index column, const Tileset &tileset) {
		for (const Layer layer: collidingLayers)
			if (std::optional<TileID> tile = tryTile(layer, {row, column}); !tile || !tileset.isWalkable(*tile) || tileset.isSolid(*tile))
				return false;
		auto lock = tileEntities.sharedLock();
		if (auto iter = tileEntities.find({row, column}); iter != tileEntities.end() && iter->second->solid)
			return false;
		return true;
	}

	void Realm::setLayerHelper(Index row, Index column, Layer layer, TileUpdateContext context) {
		if (isServer()) {
			const auto &tileset = getTileset();
			const Position position(row, column);

			{
				std::unique_lock<std::shared_mutex> path_lock;
				tileProvider.findPathState(position, &path_lock) = isWalkable(row, column, tileset);
			}

			updateNeighbors(position, layer, context);
		} else if (isActive()) {
			// queueStaticLightingTexture();
		}
	}

	Realm::ChunkPackets Realm::getChunkPackets(ChunkPosition chunk_position) {
		ChunkTilesPacket chunk_tiles(*this, chunk_position);
		std::vector<EntityPacket> entity_packets;
		std::vector<TileEntityPacket> tile_entity_packets;

		if (auto entities_ptr = getEntities(chunk_position)) {
			entity_packets.reserve(entities_ptr->size());
			for (const WeakEntityPtr &weak_entity: *entities_ptr)
				if (EntityPtr entity = weak_entity.lock())
					entity_packets.emplace_back(entity);
		}

		if (auto tile_entities_ptr = getTileEntities(chunk_position)) {
			tile_entity_packets.reserve(tile_entities_ptr->size());
			for (const auto &tile_entity: *tile_entities_ptr)
				tile_entity_packets.emplace_back(tile_entity);
		}

		return {std::move(chunk_tiles), std::move(entity_packets), std::move(tile_entity_packets)};
	}

	void Realm::remakePathMap() {
		const auto &tileset = getTileset();
		for (auto &[chunk_position, path_chunk]: tileProvider.pathMap)
			for (int64_t row = 0; row < CHUNK_SIZE; ++row)
				for (int64_t column = 0; column < CHUNK_SIZE; ++column)
					path_chunk[row * CHUNK_SIZE + column] = isWalkable(row, column, tileset);
	}

	void Realm::remakePathMap(const ChunkRange &range) {
		range.iterate([this](ChunkPosition chunk_position) {
			remakePathMap(chunk_position);
		});
	}

	void Realm::remakePathMap(ChunkPosition position) {
		Timer timer{"RemakePathMap"};
		const auto &tileset = getTileset();
		auto &path_chunk = tileProvider.getPathChunk(position);
		auto lock = path_chunk.uniqueLock();
		for (int64_t row = 0; row < CHUNK_SIZE; ++row)
			for (int64_t column = 0; column < CHUNK_SIZE; ++column)
				path_chunk[row * CHUNK_SIZE + column] = isWalkable(position.y * CHUNK_SIZE + row, position.x * CHUNK_SIZE + column, tileset);
	}

	void Realm::remakePathMap(Position position) {
		const auto &tileset = getTileset();
		auto &path_chunk = tileProvider.getPathChunk(position.getChunk());
		auto lock = path_chunk.uniqueLock();
		path_chunk[position.row * CHUNK_SIZE + position.column] = isWalkable(position.row, position.column, tileset);
	}

	void Realm::markGenerated(const ChunkRange &range) {
		for (auto y = range.topLeft.y; y <= range.bottomRight.y; ++y)
			for (auto x = range.topLeft.x; x <= range.bottomRight.x; ++x)
				generatedChunks.emplace(x, y);
	}

	void Realm::markGenerated(ChunkPosition chunk_position) {
		generatedChunks.insert(chunk_position);
	}

	bool Realm::isVisible(const Position &position) {
		const auto chunk_pos = position.getChunk();
		auto lock = players.sharedLock();
		return std::ranges::any_of(players, [chunk_pos](const auto &weak_player) {
			if (auto player = weak_player.lock()) {
				const auto player_chunk_pos = player->getPosition().getChunk();
				if (player_chunk_pos.x - REALM_DIAMETER / 2 <= chunk_pos.x && chunk_pos.x <= player_chunk_pos.x + REALM_DIAMETER / 2)
					return player_chunk_pos.y - REALM_DIAMETER / 2 <= chunk_pos.y && chunk_pos.y <= player_chunk_pos.y + REALM_DIAMETER / 2;
			}

			return false;
		});
	}

	bool Realm::hasTileEntity(GlobalID tile_entity_gid) {
		auto lock = tileEntitiesByGID.sharedLock();
		return tileEntitiesByGID.contains(tile_entity_gid);
	}

	bool Realm::hasEntity(GlobalID entity_gid) {
		auto lock = entitiesByGID.sharedLock();
		return entitiesByGID.contains(entity_gid);
	}

	EntityPtr Realm::getEntity(GlobalID entity_gid) {
		auto lock = entitiesByGID.sharedLock();
		if (auto iter = entitiesByGID.find(entity_gid); iter != entitiesByGID.end())
			return iter->second;
		return {};
	}

	TileEntityPtr Realm::getTileEntity(GlobalID tile_entity_gid) {
		auto lock = tileEntitiesByGID.sharedLock();
		if (auto iter = tileEntitiesByGID.find(tile_entity_gid); iter != tileEntitiesByGID.end())
			return iter->second;
		return {};
	}

	Side Realm::getSide() const {
		return getGame()->getSide();
	}

	std::set<ChunkPosition> Realm::getMissingChunks() const {
		assert(getSide() == Side::Client);
		std::set<ChunkPosition> out;
		auto player = getGame()->toClient().getPlayer();

		auto chunk_pos = player->getPosition().getChunk();
		chunk_pos.y -= REALM_DIAMETER / 2;
		chunk_pos.x -= REALM_DIAMETER / 2;

		const auto original_x = chunk_pos.x;

		if (!baseRenderers) {
			return out;
		}

		for (const auto &row: *baseRenderers) {
			chunk_pos.x = original_x;

			for (const auto &renderer: row) {
				if (renderer.isMissing) {
					out.insert(chunk_pos);
					break;
				}
				++chunk_pos.x;
			}

			++chunk_pos.y;
		}

		return out;
	}

	void Realm::addPlayer(const PlayerPtr &player) {
		auto players_lock = players.uniqueLock();
		players.insert(player);
		recalculateVisibleChunks();
	}

	void Realm::removePlayer(const PlayerPtr &player) {
		auto players_lock = players.uniqueLock();
		players.erase(player);
		if (players.empty()) {
			auto lock = visibleChunks.uniqueLock();
			visibleChunks.clear();
		} else
			recalculateVisibleChunks();
	}

	void Realm::sendTo(RemoteClient &client) {
		auto player = client.getPlayer();
		assert(player);

		player->notifyOfRealm(*this);

		for (const auto &chunk_position: player->getVisibleChunks())
			client.sendChunk(*this, chunk_position);

		auto guard = client.bufferGuard();

		{
			auto lock = entities.sharedLock();
			for (const auto &entity: entities)
				if (player->canSee(*entity))
					entity->sendTo(client);
		}

		{
			auto lock = tileEntities.sharedLock();
			for (const auto &[tile_position, tile_entity]: tileEntities)
				if (player->canSee(*tile_entity))
					tile_entity->sendTo(client);
		}
	}

	void Realm::requestChunk(ChunkPosition chunk_position, const std::shared_ptr<RemoteClient> &client) {
		assert(isServer());
		tileProvider.generationQueue.push(chunk_position);
		auto lock = chunkRequests.uniqueLock();
		chunkRequests[chunk_position].insert(client);
	}

	void Realm::detach(const EntityPtr &entity, ChunkPosition chunk_position) {
		auto lock = entitiesByChunk.uniqueLock();

		if (auto iter = entitiesByChunk.find(chunk_position); iter != entitiesByChunk.end()) {
			auto sublock = iter->second->uniqueLock();

			if (0 < iter->second->erase(entity)) {
				if (iter->second->empty()) {
					sublock.unlock();
					sublock.release();
					entitiesByChunk.erase(iter);
				}
			}
		}
	}

	void Realm::detach(const EntityPtr &entity) {
		detach(entity, entity->getChunk());
	}

	void Realm::attach(const EntityPtr &entity) {
		auto lock = entitiesByChunk.uniqueLock();
		const auto chunk_position = entity->getChunk();

		if (auto iter = entitiesByChunk.find(chunk_position); iter != entitiesByChunk.end()) {
			assert(iter->second);
			auto &set = *iter->second;
			auto set_lock = set.uniqueLock();
			set.insert(entity);
		} else {
			auto set = std::make_shared<WeakEntitySet::element_type>();
			set->insert(entity);
			entitiesByChunk.emplace(chunk_position, std::move(set));
		}
	}

	Realm::WeakEntitySet Realm::getEntities(ChunkPosition chunk_position) const {
		auto lock = entitiesByChunk.sharedLock();
		if (auto iter = entitiesByChunk.find(chunk_position); iter != entitiesByChunk.end())
			return iter->second;
		return {};
	}

	void Realm::detach(const TileEntityPtr &tile_entity) {
		auto lock = tileEntitiesByChunk.uniqueLock();
		if (auto iter = tileEntitiesByChunk.find(tile_entity->getChunk()); iter != tileEntitiesByChunk.end()) {
			iter->second->erase(tile_entity);
			if (iter->second->empty())
				tileEntitiesByChunk.erase(iter);
		}
	}

	void Realm::addToMaps(const TileEntityPtr &tile_entity) {
		std::scoped_lock lock{tileEntities.mutex, tileEntitiesByGID.mutex};
		tileEntities.emplace(tile_entity->getPosition(), tile_entity);
		tileEntitiesByGID.emplace(tile_entity->globalID, tile_entity);
	}

	void Realm::attach(const TileEntityPtr &tile_entity) {
		// TODO: consider adding a call to addToMaps here
		auto shared_lock = tileEntitiesByChunk.sharedLock();
		const auto chunk_position = tile_entity->getChunk();
		if (auto iter = tileEntitiesByChunk.find(chunk_position); iter != tileEntitiesByChunk.end()) {
			assert(iter->second);
			auto set = iter->second;
			auto set_lock = set->uniqueLock();
			set->insert(tile_entity);
		} else {
			auto set = std::make_shared<Lockable<std::unordered_set<TileEntityPtr>>>();
			set->insert(tile_entity);
			shared_lock.unlock();
			auto unique_lock = tileEntitiesByChunk.uniqueLock();
			tileEntitiesByChunk.emplace(chunk_position, std::move(set));
		}
	}

	std::shared_ptr<Lockable<std::unordered_set<TileEntityPtr>>> Realm::getTileEntities(ChunkPosition chunk_position) {
		auto lock = tileEntitiesByChunk.sharedLock();
		if (auto iter = tileEntitiesByChunk.find(chunk_position); iter != tileEntitiesByChunk.end())
			return iter->second;
		return {};
	}

	void Realm::sendToMany(const std::unordered_set<std::shared_ptr<RemoteClient>> &clients, ChunkPosition chunk_position) {
		assert(getSide() == Side::Server);

		if (clients.empty())
			return;

		try {
			const auto [chunk_tiles, entity_packets, tile_entity_packets] = getChunkPackets(chunk_position);

			for (const auto &client: clients) {
				client->getPlayer()->notifyOfRealm(*this);
				client->send(chunk_tiles);
				for (const auto &packet: entity_packets)
					client->send(packet);
				for (const auto &packet: tile_entity_packets)
					client->send(packet);
			}
		} catch (const std::out_of_range &) {
			const ErrorPacket packet("Chunk " + static_cast<std::string>(chunk_position) + " not present in realm " + std::to_string(id));
			for (const auto &client: clients)
				client->send(packet);
			return;
		}
	}

	void Realm::sendToOne(RemoteClient &client, ChunkPosition chunk_position) {
		if (ServerPlayerPtr player = client.getPlayer()) {
			const auto [chunk_tiles, entity_packets, tile_entity_packets] = getChunkPackets(chunk_position);
			player->notifyOfRealm(*this);
			client.send(chunk_tiles);
			for (const auto &packet: entity_packets)
				client.send(packet);
			for (const auto &packet: tile_entity_packets)
				client.send(packet);
		}
	}

	void Realm::recalculateVisibleChunks() {
		decltype(visibleChunks)::Base new_visible_chunks;
		for (const auto &weak_player: players) {
			if (auto player = weak_player.lock()) {
				ChunkRange(player->getChunk()).iterate([&](ChunkPosition chunk_position) {
					new_visible_chunks.insert(chunk_position);
				});
			}
		}
		visibleChunks = std::move(new_visible_chunks);
	}

	void Realm::queueReupload() {
		assert(getSide() == Side::Client);
		if (!reuploadPending.exchange(true)) {
			getGame()->toClient().getWindow().queue([weak = std::weak_ptr(shared_from_this())] {
				if (auto shared = weak.lock()) {
					shared->reupload();
					shared->reuploadPending = false;
				} else {
					ERROR("Expired in {}:{}", __FILE__, __LINE__);
				}
			});
		}
	}

	void Realm::autotile(const Position &position, Layer layer, TileUpdateContext context) {
		const Tileset &tileset = getTileset();
		const TileID tile = tileProvider.copyTile(layer, position, TileProvider::TileMode::ReturnEmpty);
		const Identifier &tilename = tileset[tile];

		if (const MarchableInfo *info = tileset.getMarchableInfo(tilename)) {
			const std::unordered_set<Identifier> &members = info->autotileSet->members;
			TileID march_result{};

			if (info->autotileSet->omni) {
				const TileID empty = tileset.getEmptyID();
				march_result = march4([&](int8_t march_row_offset, int8_t march_column_offset) -> bool {
					const Position march_position = position + Position(march_row_offset, march_column_offset);
					for (const Layer omni_layer: {Layer::Submerged, Layer::Objects}) {
						const TileID march_tile = tileProvider.copyTile(omni_layer, march_position, TileProvider::TileMode::ReturnEmpty);
						if (march_tile != empty && !tileset.isInCategory(march_tile, "base:category/no_omni"))
							return true;
					}
					return false;
				});
			} else {
				march_result = march4([&](int8_t march_row_offset, int8_t march_column_offset) -> bool {
					const Position march_position = position + Position(march_row_offset, march_column_offset);
					const TileID march_tile = tileProvider.copyTile(layer, march_position, TileProvider::TileMode::ReturnEmpty);
					return members.contains(tileset[march_tile]);
				});
			}

			const TileID marched = tileset[info->start] + (info->tall? 2 * march_result : march_result);

			if (marched != tile) {
				setTile(layer, position, marched, true, context);
				threadContext.updatedLayers.insert(layer);
			}
		}
	}

	void Realm::remakeStaticLightingTexture() {
		assert(isClient());
		ClientGame &game = getGame()->toClient();
		PlayerPtr player = game.getPlayer();

		if (!player)
			return;

		Timer timer("RemakeStaticLightingTexture");
		Canvas &canvas = game.canvas;
		GL::Texture &texture = canvas.staticLightingTexture;
		GL::FBOBinder binder = canvas.fbo.getBinder();
		GL::TextureFBOBinder texture_binder = texture.getBinder();
		GL::clear(0, 0, 0, 0);
		const ChunkPosition chunk = player->getChunk();

		const auto [top,     left] = (chunk - ChunkPosition(1, 1)).topLeft();
		const auto [bottom, right] = (chunk + ChunkPosition(1, 1)).bottomRight();

		Tileset &tileset = getTileset();
		RealmPtr shared = shared_from_this();
		RendererContext context = canvas.getRendererContext();
		auto saver = context.getSaver();
		context.updateSize(texture.getWidth(), texture.getHeight());
		GL::Viewport viewport(0, 0, texture.getWidth(), texture.getHeight());

		for (Index row = top; row <= bottom; ++row) {
			for (Index column = left; column <= right; ++column) {
				Place place{Position(row, column), shared, player};
				for (const Layer layer: allLayers) {
					if (std::optional<TileID> tile_id = tryTile(layer, place.position)) {
						std::shared_ptr<Tile> tile = game.getTile(tileset[*tile_id]);
						tile->renderStaticLighting(place, layer, context);
					}
				}
			}
		}

		viewport.reset();
	}

	void Realm::queueStaticLightingTexture() {
		if (isClient())
			staticLightingQueued = true;
	}

	void Realm::playSound(const Position &position, const Identifier &id, float pitch) const {
		assert(getSide() == Side::Server);
		PlaySoundPacket packet(id, position, pitch);
		getPlayers().withShared([&](const WeakSet<Player> &set) {
			for (const auto &weak_player: set)
				if (auto player = weak_player.lock())
					player->send(packet);
		});
	}

	bool Realm::rightClick(const Position &position, double x, double y) {
		if (getSide() != Side::Client)
			return false;

		ClientGame &game = getGame()->toClient();
		const auto player     = game.getPlayer();
		const auto player_pos = player->getPosition();
		const bool overlap    = player_pos == position;
		const bool adjacent   = position.adjacent4(player_pos);

		if (!overlap && !adjacent)
			return false;

		auto gmenu = Gio::Menu::create();
		auto group = Gio::SimpleActionGroup::create();

		bool show_menu = false;
		size_t i = 0;

		auto add = [&](const AgentPtr &agent) {
			show_menu = true;

			if (!agent->populateMenu(player, overlap, std::to_string(i), gmenu, group)) {
				// TODO: Can you escape underscores?
				gmenu->append(agent->getName(), "agent_menu.agent" + std::to_string(i));
				group->add_action("agent" + std::to_string(i), [agent, overlap, player] {
					player->send(InteractPacket(overlap, Hand::None, Modifiers{}, agent->getGID(), player->direction));
				});
			}

			++i;
		};

		if (const TileEntityPtr tile_entity = tileEntityAt(position))
			add(tile_entity);

		if (const auto found = findEntities(position); !found.empty())
			for (const EntityPtr &entity: found)
				add(entity);

		if (show_menu) {
			MainWindow &window = game.getWindow();
			auto &menu = window.getGLMenu();
			window.remove_action_group("agent_menu");
			window.insert_action_group("agent_menu", group);
			menu.set_menu_model(gmenu);
			menu.set_has_arrow(true);
			menu.set_pointing_to({static_cast<int>(x), static_cast<int>(y), 1, 1});
			menu.popup();
			return true;
		}

		return false;
	}

	bool Realm::canSpawnMonsters() const {
		if (!outdoors)
			return false;

		const double hour = getGame()->getHour();
		return !(5. <= hour && hour < 21.);
	}

	void Realm::initEntity(const EntityPtr &entity, const Position &position) {
		GamePtr game = getGame();
		entity->init(game);
		add(entity, position);
		entity->calculateVisibleEntities();
		entity->spawning = false;
		entity->onSpawn();

		if (getSide() == Side::Server) {
			auto lock = entity->visiblePlayers.sharedLock();
			if (!entity->visiblePlayers.empty()) {
				const EntityPacket packet(entity);
				for (const auto &weak_player: entity->visiblePlayers) {
					if (auto player = weak_player.lock()) {
						player->notifyOfRealm(*this);
						player->send(packet);
					}
				}
			}
		}

		entity->enqueueTick();
	}

	bool Realm::isActive() const {
		assert(isClient());
		GamePtr game = getGame();
		return game->toClient().getActiveRealm().get() == this;
	}

	BiomeType Realm::getBiome(int64_t seed) {
		std::default_random_engine rng{static_cast<uint_fast32_t>(seed * 79ul)};
		return std::uniform_int_distribution(BiomeType(1), Biome::COUNT)(rng);
	}
}
