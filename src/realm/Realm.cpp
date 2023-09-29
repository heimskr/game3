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
#include "graphics/SpriteRenderer.h"
#include "graphics/Tileset.h"
#include "net/RemoteClient.h"
#include "packet/ErrorPacket.h"
#include "realm/Keep.h"
#include "realm/Realm.h"
#include "realm/RealmFactory.h"
#include "threading/ThreadContext.h"
#include "tile/Tile.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Carpet.h"
#include "worldgen/House.h"
#include "worldgen/Keep.h"

#include <iostream>
#include <thread>
#include <unordered_set>

namespace Game3 {
	void from_json(const nlohmann::json &json, RealmDetails &details) {
		details.tilesetName = json.at("tileset");
	}

	Realm::Realm(Game &game_): game(game_) {
		if (game.getSide() == Side::Client) {
			game.toClient().getWindow().queue([this] {
				createRenderers();
				initRendererRealms();
				initRendererTileProviders();
				renderersReady = true;
			});
		}
	}

	Realm::Realm(Game &game_, RealmID id_, RealmType type_, Identifier tileset_id, int64_t seed_):
	id(id_), type(std::move(type_)), tileProvider(std::move(tileset_id)), seed(seed_), game(game_) {
		if (game.getSide() == Side::Client) {
			game.toClient().getWindow().queue([this] {
				createRenderers();
				initRendererRealms();
				initTexture();
				initRendererTileProviders();
				renderersReady = true;
			});
		}
	}

	void Realm::initRendererRealms() {
		for (auto &row: *renderers)
			for (auto &layers: row)
				for (auto &renderer: layers)
					renderer.setRealm(*this);

		for (auto &row: *fluidRenderers)
			for (auto &renderer: row)
				renderer.setRealm(*this);
	}

	void Realm::initRendererTileProviders() {
		for (auto &row: *renderers) {
			for (auto &layers: row) {
				size_t layer = 0;
				for (auto &renderer: layers)
					renderer.setup(tileProvider, getLayer(++layer));
			}
		}

		for (auto &row: *fluidRenderers)
			for (auto &renderer: row)
				renderer.setup(tileProvider);
	}

	void Realm::initTexture() {}

	RealmPtr Realm::fromJSON(Game &game, const nlohmann::json &json, bool full_data) {
		const RealmType type = json.at("type");
		auto factory = game.registry<RealmFactoryRegistry>().at(type);
		assert(factory);
		auto out = (*factory)(game);
		out->absorbJSON(json, full_data);
		return out;
	}

	void Realm::absorbJSON(const nlohmann::json &json, bool full_data) {
		auto shared = shared_from_this();
		id = json.at("id");
		type = json.at("type");
		seed = json.at("seed");
		generatedChunks = json.at("generatedChunks");
		tileProvider.clear();

		if (json.contains("extra"))
			extraData = json.at("extra");

		initRendererTileProviders();
		initTexture();

		tileProvider.absorbJSON(json.at("provider"), full_data);

		if (full_data) {
			outdoors = json.at("outdoors");

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

	void Realm::onFocus() {
		if (getSide() != Side::Client || focused)
			return;

		focused = true;
		wakeupPending = true;
	}

	void Realm::onBlur() {
		if (getSide() != Side::Client || !focused)
			return;

		focused = false;
		snoozePending = true;
	}

	void Realm::createRenderers() {
		if (getSide() != Side::Client)
			return;

		game.toClient().activateContext();
		renderers.emplace();
		fluidRenderers.emplace();
	}

	void Realm::render(const int width, const int height, const Eigen::Vector2f &center, float scale, SpriteRenderer &sprite_renderer, TextRenderer &text_renderer, float game_time) {
		if (getSide() != Side::Client)
			return;

		if (!focused)
			onFocus();

		auto &client_game = game.toClient();
		// Canvas &canvas = client_game.canvas;
		// auto &multiplier = canvas.multiplier;

		const auto bb_width  = width;
		const auto bb_height = height;

		const auto &visible = client_game.player? client_game.player->getVisibleLayers() : std::unordered_set{Layer::Terrain, Layer::Submerged, Layer::Objects, Layer::Highest};

		if (renderers) {
			for (auto &row: *renderers) {
				for (auto &layers: row) {
					uint8_t layer = 0;
					for (auto &renderer: layers) {
						if (visible.contains(static_cast<Layer>(++layer))) {
							renderer.onBackbufferResized(bb_width, bb_height);
							renderer.render(outdoors? game_time : 1, scale, center.x(), center.y());
						}
					}
				}
			}
		}

		if (fluidRenderers) {
			for (auto &row: *fluidRenderers) {
				for (auto &renderer: row) {
					renderer.onBackbufferResized(bb_width, bb_height);
					renderer.render(outdoors? game_time : 1, scale, center.x(), center.y());
				}
			}
		}

		sprite_renderer.centerX = center.x();
		sprite_renderer.centerY = center.y();
		sprite_renderer.update(bb_width, bb_height);
		sprite_renderer.divisor = outdoors? game_time : 1;
		text_renderer.centerX = center.x();
		text_renderer.centerY = center.y();
		text_renderer.update(bb_width, bb_height);

		{
			auto lock = tileEntities.sharedLock();
			for (const auto &[index, tile_entity]: tileEntities)
				tile_entity->render(sprite_renderer);
		}

		{
			auto lock = entities.sharedLock();
			for (const auto &entity: entities)
				entity->render(sprite_renderer, text_renderer);
		}

		// multiplier.update(bb_width, bb_height);
		// sprite_renderer.drawOnMap(texture, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f, 1.f);
		// if (renderer1.lightTexture) {
			// textureB.useInFB();
			// multiplier(textureA, renderer1.lightTexture);
			// textureA.useInFB();
			// game.canvas.multiplier(textureB, renderer2.lightTexture);
		// }
		// textureB.useInFB();
		// game.canvas.multiplier(textureA, renderer3.lightTexture);
		// sprite_renderer.drawOnScreen(renderer1.lightTexture, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f);
		// sprite_renderer.drawOnScreen(renderer2.lightTexture, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f);
		// sprite_renderer.drawOnScreen(renderer3.lightTexture, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f);

		// fbo.undo();
		// viewport.reset();

		// GL::clear(1.f, 0.f, 1.f, 0.f);

		// // sprite_renderer.update(width, height);
		// // sprite_renderer.drawOnMap(textureB, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f, 1.f);
		// sprite_renderer.drawOnScreen(textureA, {
		// 	// .x = -center.x() / 2.f,
		// 	// .y = -0.5f,
		// 	// .y = -center.y() / 2.f,
		// 	.sizeX = -1.f,
		// 	.sizeY = -1.f,
		// });
	}

	void Realm::reupload() {
		if (getSide() != Side::Client)
			return;

		getGame().toClient().activateContext();
		for (auto &row: *renderers)
			for (auto &layers: row)
				for (auto &renderer: layers)
					renderer.reupload();
	}

	void Realm::reupload(Layer layer) {
		if (getSide() != Side::Client)
			return;

		getGame().toClient().activateContext();
		for (auto &row: *renderers)
			for (auto &layers: row)
				layers[getIndex(layer)].reupload();
	}

	void Realm::reuploadFluids() {
		if (getSide() != Side::Client)
			return;

		getGame().toClient().activateContext();
		for (auto &row: *fluidRenderers)
			for (auto &renderer: row)
				renderer.reupload();
	}

	EntityPtr Realm::addUnsafe(const EntityPtr &entity, const Position &position) {
		if (auto found = getEntity(entity->getGID()))
			return found;
		auto shared = shared_from_this();
		entities.insert(entity);
		entitiesByGID[entity->globalID] = entity;
		entity->firstTeleport = true;
		if (entity->isPlayer() && entity->weakRealm.lock())
			std::static_pointer_cast<Player>(entity)->stopMoving();
		entity->setRealm(shared);
		entity->teleport(position, MovementContext{.excludePlayerSelf = true, .isTeleport = true});
		entity->firstTeleport = false;
		attach(entity);
		if (entity->isPlayer()) {
			{
				auto lock = players.uniqueLock();
				players.emplace(std::static_pointer_cast<Player>(entity));
			}
			recalculateVisibleChunks();
		}
		return entity;
	}

	EntityPtr Realm::add(const EntityPtr &entity, const Position &position) {
		auto lock = entities.uniqueLock();
		return addUnsafe(entity, position);
	}

	TileEntityPtr Realm::addUnsafe(const TileEntityPtr &tile_entity) {
		if (tileEntities.contains(tile_entity->position))
			return nullptr;
		tile_entity->setRealm(shared_from_this());
		if (!tile_entity->initialized)
			tile_entity->init(game);
		tileEntities.emplace(tile_entity->position, tile_entity);
		tileEntitiesByGID[tile_entity->globalID] = tile_entity;
		attach(tile_entity);
		if (tile_entity->solid) {
			std::unique_lock<std::shared_mutex> path_lock;
			tileProvider.findPathState(tile_entity->position.copyBase(), &path_lock) = 0;
		}
		tile_entity->onSpawn();
		return tile_entity;
	}

	TileEntityPtr Realm::add(const TileEntityPtr &tile_entity) {
		auto lock = tileEntities.uniqueLock();
		return addUnsafe(tile_entity);
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
		ticking = true;

		for (const auto &[entity, position]: entityInitializationQueue.steal()) {
			initEntity(entity, position);
		}

		for (const auto &[entity, position]: entityAdditionQueue.steal())
			add(entity, position);

		for (const auto &stolen: tileEntityAdditionQueue.steal())
			if (auto locked = stolen.lock())
				add(locked);

		if (isServer()) {
			std::vector<RemoteClient::BufferGuard> guards;

			{
				auto lock = players.sharedLock();
				guards.reserve(players.size());
				for (const auto &weak_player: players) {
					if (auto player = weak_player.lock()) {
						if (auto client = player->toServer()->weakClient.lock())
							guards.emplace_back(client);

						if (!player->ticked) {
							player->ticked = true;
							player->tick(game, delta);
						}
					}
				}
			}

			{
				auto visible_lock = visibleChunks.sharedLock();
				for (const auto &chunk: visibleChunks) {
					{
						auto by_chunk_lock = entitiesByChunk.sharedLock();
						if (auto iter = entitiesByChunk.find(chunk); iter != entitiesByChunk.end() && iter->second)
							for (const auto &entity: *iter->second)
								if (!entity->isPlayer())
									entity->tick(game, delta);
					}
					{
						auto by_chunk_lock = tileEntitiesByChunk.sharedLock();
						if (auto iter = tileEntitiesByChunk.find(chunk); iter != tileEntitiesByChunk.end() && iter->second)
							for (const auto &tile_entity: *iter->second)
								tile_entity->tick(game, delta);
					}
					static std::uniform_int_distribution<int64_t> distribution(0l, CHUNK_SIZE - 1);
					auto &tileset = getTileset();
					auto shared = shared_from_this();

					for (size_t i = 0; i < game.randomTicksPerChunk; ++i) {
						const Position position(chunk.y * CHUNK_SIZE + distribution(threadContext.rng), chunk.x * CHUNK_SIZE + distribution(threadContext.rng));

						for (const Layer layer: mainLayers)
							if (auto tile_id = tileProvider.tryTile(layer, position); tile_id && *tile_id != 0)
								game.getTile(tileset[*tile_id])->randomTick({position, shared, nullptr});
					}
				}
			}

			ticking = false;

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

			auto player = getGame().toClient().player;
			if (!player)
				return;

			const auto player_cpos = getChunkPosition(player->getPosition());

			{
				auto lock = entities.sharedLock();
				for (const auto &entity: entities)
					entity->tick(game, delta);
			}

			{
				auto lock = tileEntities.sharedLock();
				for (auto &[index, tile_entity]: tileEntities)
					tile_entity->tick(game, delta);
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
				for (auto &row: *renderers) {
					Index col_index = 0;
					for (auto &layers: row) {
						for (auto &renderer: layers) {
							renderer.setChunkPosition({
								static_cast<int32_t>(player_cpos.x + col_index - REALM_DIAMETER / 2 - 1),
								static_cast<int32_t>(player_cpos.y + row_index - REALM_DIAMETER / 2 - 1),
							});
						}
						++col_index;
					}
					++row_index;
				}

				row_index = 0;
				for (auto &row: *fluidRenderers) {
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
	}

	std::vector<EntityPtr> Realm::findEntities(const Position &position) {
		std::vector<EntityPtr> out;
		auto lock = entities.sharedLock();
		for (const auto &entity: entities)
			if (entity->position == position)
				out.push_back(entity);
		return out;
	}

	std::vector<EntityPtr> Realm::findEntities(const Position &position, const EntityPtr &except) {
		std::vector<EntityPtr> out;
		auto lock = entities.sharedLock();
		for (const auto &entity: entities)
			if (entity->position == position && entity != except)
				out.push_back(entity);
		return out;
	}

	EntityPtr Realm::findEntity(const Position &position) {
		auto lock = entities.sharedLock();
		for (const auto &entity: entities)
			if (entity->position == position)
				return entity;
		return {};
	}

	EntityPtr Realm::findEntity(const Position &position, const EntityPtr &except) {
		auto lock = entities.sharedLock();
		for (const auto &entity: entities)
			if (entity->position == position && entity != except)
				return entity;
		return {};
	}

	TileEntityPtr Realm::tileEntityAt(const Position &position) {
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
			WARN("Tile entity use count: " << count);

		if (run_helper) {
			setLayerHelper(position.row, position.column, Layer::Submerged, false);
			setLayerHelper(position.row, position.column, Layer::Objects, false);
		}

		updateNeighbors(position, Layer::Submerged);
		updateNeighbors(position, Layer::Objects);
	}

	void Realm::removeSafe(const TileEntityPtr &tile_entity) {
		auto lock = tileEntities.uniqueLock();
		remove(tile_entity, true);
	}

	void Realm::onMoved(const EntityPtr &entity, const Position &position) {
		if (auto tile_entity = tileEntityAt(position))
			tile_entity->onOverlap(entity);
	}

	Game & Realm::getGame() {
		return game;
	}

	const Game & Realm::getGame() const {
		return game;
	}

	void Realm::queueRemoval(const EntityPtr &entity) {
		entityRemovalQueue.push(entity);
	}

	void Realm::queueRemoval(const TileEntityPtr &tile_entity) {
		tileEntityRemovalQueue.push(tile_entity);
	}

	void Realm::queueDestruction(const EntityPtr &entity) {
		if (entity->isPlayer())
			INFO("Queueing player " << entity->getGID() << " for entity destruction.");
		entityDestructionQueue.push(entity);
	}

	void Realm::queueDestruction(const TileEntityPtr &tile_entity) {
		tileEntityDestructionQueue.push(tile_entity);
	}

	void Realm::queuePlayerRemoval(const PlayerPtr &player) {
		INFO("Queueing player " << player->getGID() << " for player removal.");
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
		entity->init(getGame());
		entity->teleport(position);
	}

	void Realm::setTile(Layer layer, Index row, Index column, TileID tile_id, bool run_helper) {
		setTile(layer, Position(row, column), tile_id, run_helper);
	}

	void Realm::setTile(Layer layer, const Position &position, TileID tile_id, bool run_helper) {
		{
			std::unique_lock<std::shared_mutex> tile_lock;
			auto &tile = tileProvider.findTile(layer, position, &tile_lock, TileProvider::TileMode::Create);
			if (tile == tile_id)
				return;
			tile = tile_id;
		}

		if (isServer()) {
			if (!isGenerating()) {
				tileProvider.updateChunk(getChunkPosition(position));
				getGame().toServer().broadcastTileUpdate(id, layer, position, tile_id);
			}
			if (run_helper)
				setLayerHelper(position.row, position.column, layer);
		}
	}

	void Realm::setTile(Layer layer, const Position &position, const Identifier &tilename, bool run_helper) {
		setTile(layer, position, getTileset()[tilename], run_helper);
	}

	void Realm::setFluid(const Position &position, FluidTile tile) {
		{
			std::unique_lock<std::shared_mutex> fluid_lock;
			auto &fluid = tileProvider.findFluid(position, &fluid_lock);
			if (fluid == tile)
				return;
			fluid = tile;
		}

		if (isServer() && !isGenerating()) {
			tileProvider.updateChunk(getChunkPosition(position));
			getGame().toServer().broadcastFluidUpdate(id, position, tile);
		}
	}

	void Realm::setFluid(const Position &position, const Identifier &fluidname, FluidLevel level) {
		auto fluid = getGame().registry<FluidRegistry>().at(fluidname);
		assert(fluid);
		setFluid(position, FluidTile(fluid->registryID, level));
	}

	bool Realm::hasFluid(const Position &position, FluidLevel minimum) {
		if (auto fluid = tileProvider.copyFluidTile(position))
			return minimum <= fluid->level;
		return false;
	}

	TileID Realm::getTile(Layer layer, const Position &position) const {
		return tileProvider.copyTile(layer, position, TileProvider::TileMode::Throw);
	}

	bool Realm::middleEmpty(const Position &position) {
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

	bool Realm::interactGround(const PlayerPtr &player, const Position &position, Modifiers modifiers) {
		const Place place(position, shared_from_this(), player);
		auto &game = getGame();

		if (auto iter = game.interactionSets.find(type); iter != game.interactionSets.end())
			if (iter->second->interact(place, modifiers))
				return true;

		auto &tileset = getTileset();

		// TODO: when normally invisible layers become conditionally visible, handle that here.
		for (const Layer layer: reverse(mainLayers))
			if (auto tile = tryTile(layer, position))
				if (game.getTile(tileset[*tile])->interact(place, layer))
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

	void Realm::updateNeighbors(const Position &position, Layer layer) {
		if (updatesPaused)
			return;

		++threadContext.updateNeighborsDepth;

		for (Index row_offset = -1; row_offset <= 1; ++row_offset) {
			for (Index column_offset = -1; column_offset <= 1; ++column_offset) {
				if (row_offset != 0 || column_offset != 0) {
					const Position offset_position = position + Position(row_offset, column_offset);
					if (auto neighbor = tileEntityAt(offset_position))
						neighbor->onNeighborUpdated(Position(-row_offset, -column_offset));
					autotile(offset_position, layer);
				}
			}
		}

		if (--threadContext.updateNeighborsDepth == 0) {
			for (const Layer layer: threadContext.updatedLayers)
				reupload(layer);
			threadContext.updatedLayers.clear();
		}
	}

	bool Realm::hasTileEntityAt(const Position &position) const {
		return tileEntities.contains(position);
	}

	void Realm::damageGround(const Position &position) {
		const Place place(position, shared_from_this(), nullptr);
		auto &game = getGame();

		if (auto iter = game.interactionSets.find(type); iter != game.interactionSets.end())
			iter->second->damageGround(place);
	}

	Tileset & Realm::getTileset() {
		return *tileProvider.getTileset(getGame());
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
			json["tileEntities"] = std::unordered_map<std::string, nlohmann::json>();
			for (const auto &[position, tile_entity]: tileEntities)
				json["tileEntities"][position.simpleString()] = *tile_entity;
			json["entities"] = std::vector<nlohmann::json>();
			for (const auto &entity: entities) {
				nlohmann::json entity_json;
				entity->toJSON(entity_json);
				json["entities"].push_back(std::move(entity_json));
			}
		}
	}

	bool Realm::isWalkable(Index row, Index column, const Tileset &tileset) {
		for (const Layer layer: mainLayers)
			if (std::optional<TileID> tile = tryTile(layer, {row, column}); !tile || !tileset.isWalkable(*tile) || tileset.isSolid(*tile))
				return false;
		auto lock = tileEntities.sharedLock();
		if (auto iter = tileEntities.find({row, column}); iter != tileEntities.end() && iter->second->solid)
			return false;
		return true;
	}

	void Realm::setLayerHelper(Index row, Index column, Layer layer, bool should_mark_dirty) {
		const auto &tileset = getTileset();
		const Position position(row, column);

		{
			std::unique_lock<std::shared_mutex> path_lock;
			tileProvider.findPathState(position, &path_lock) = isWalkable(row, column, tileset);
		}

		updateNeighbors(position, layer);

		if (should_mark_dirty)
			for (auto &row: *renderers)
				for (auto &layers: row)
					for (auto &renderer: layers)
						renderer.markDirty();
	}

	Realm::ChunkPackets Realm::getChunkPackets(ChunkPosition chunk_position) {
		ChunkTilesPacket chunk_tiles(*this, chunk_position);
		std::vector<EntityPacket> entity_packets;
		std::vector<TileEntityPacket> tile_entity_packets;

		if (auto entities_ptr = getEntities(chunk_position)) {
			entity_packets.reserve(entities_ptr->size());
			for (const auto &entity: *entities_ptr)
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
		auto &path_chunk = tileProvider.getPathChunk(getChunkPosition(position));
		auto lock = path_chunk.uniqueLock();
		path_chunk[position.row * CHUNK_SIZE + position.column] = isWalkable(position.row, position.column, tileset);
	}

	void Realm::markGenerated(const ChunkRange &range) {
		for (auto y = range.topLeft.y; y <= range.bottomRight.y; ++y)
			for (auto x = range.topLeft.x; x <= range.bottomRight.x; ++x)
				generatedChunks.emplace(x, y);
	}

	void Realm::markGenerated(ChunkPosition chunk_position) {
		generatedChunks.insert(std::move(chunk_position));
	}

	bool Realm::isVisible(const Position &position) {
		const auto chunk_pos = getChunkPosition(position);
		auto lock = players.sharedLock();
		for (const auto &weak_player: players) {
			if (auto player = weak_player.lock()) {
				const auto player_chunk_pos = getChunkPosition(player->getPosition());
				if (player_chunk_pos.x - REALM_DIAMETER / 2 <= chunk_pos.x && chunk_pos.x <= player_chunk_pos.x + REALM_DIAMETER / 2)
					if (player_chunk_pos.y - REALM_DIAMETER / 2 <= chunk_pos.y && chunk_pos.y <= player_chunk_pos.y + REALM_DIAMETER / 2)
						return true;
			}
		}

		return false;
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
		return getGame().getSide();
	}

	std::set<ChunkPosition> Realm::getMissingChunks() const {
		assert(getSide() == Side::Client);
		std::set<ChunkPosition> out;
		const auto &player = getGame().toClient().player;

		auto chunk_pos = getChunkPosition(player->getPosition());
		chunk_pos.y -= REALM_DIAMETER / 2;
		chunk_pos.x -= REALM_DIAMETER / 2;

		const auto original_x = chunk_pos.x;

		for (const auto &row: *renderers) {
			chunk_pos.x = original_x;

			for (const auto &layers: row) {
				for (const auto &renderer: layers) {
					if (renderer.isMissing) {
						out.insert(chunk_pos);
						break;
					}
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
			std::optional<std::unique_lock<std::shared_mutex>> sublock = iter->second->uniqueLock();
			if (0 < iter->second->erase(entity)) {
				if (iter->second->empty()) {
					sublock.reset();
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
			auto set = std::make_shared<Lockable<std::unordered_set<EntityPtr>>>();
			set->insert(entity);
			entitiesByChunk.emplace(chunk_position, std::move(set));
		}
	}

	std::shared_ptr<Lockable<std::unordered_set<EntityPtr>>> Realm::getEntities(ChunkPosition chunk_position) {
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

	void Realm::attach(const TileEntityPtr &tile_entity) {
		auto lock = tileEntitiesByChunk.uniqueLock();
		const auto chunk_position = tile_entity->getChunk();
		if (auto iter = tileEntitiesByChunk.find(chunk_position); iter != tileEntitiesByChunk.end()) {
			assert(iter->second);
			auto &set = *iter->second;
			auto set_lock = set.uniqueLock();
			set.insert(tile_entity);
		} else {
			auto set = std::make_shared<Lockable<std::unordered_set<TileEntityPtr>>>();
			set->insert(tile_entity);
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
		const auto [chunk_tiles, entity_packets, tile_entity_packets] = getChunkPackets(chunk_position);

		client.getPlayer()->notifyOfRealm(*this);
		client.send(chunk_tiles);
		for (const auto &packet: entity_packets)
			client.send(packet);
		for (const auto &packet: tile_entity_packets)
			client.send(packet);
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
			getGame().toClient().getWindow().queue([shared = shared_from_this()] {
				shared->reupload();
				shared->reuploadPending = false;
			});
		}
	}

	void Realm::queueReupload(Layer layer) {
		assert(getSide() == Side::Client);
		if (!layerReuploadPending[getIndex(layer)].exchange(true)) {
			getGame().toClient().getWindow().queue([shared = shared_from_this(), layer] {
				shared->reupload(layer);
				shared->layerReuploadPending[getIndex(layer)] = false;
			});
		}
	}

	void Realm::queueReuploadFluids() {
		assert(getSide() == Side::Client);
		if (!fluidReuploadPending.exchange(true)) {
			getGame().toClient().getWindow().queue([shared = shared_from_this()] {
				shared->reuploadFluids();
				shared->fluidReuploadPending = false;
			});
		}
	}

	void Realm::queueReuploadAll() {
		assert(getSide() == Side::Client);
		const bool fluid_reuploading = fluidReuploadPending.exchange(true);
		const bool reuploading = reuploadPending.exchange(true);
		if (!fluid_reuploading && !reuploading) {
			getGame().toClient().getWindow().queue([shared = shared_from_this()] {
				shared->reupload();
				shared->reuploadFluids();
				shared->reuploadPending = false;
				shared->fluidReuploadPending = false;
			});
		} else if (fluid_reuploading && !reuploading) {
			getGame().toClient().getWindow().queue([shared = shared_from_this()] {
				shared->reupload();
				shared->reuploadPending = false;
			});
		} else if (!fluid_reuploading && reuploading) {
			getGame().toClient().getWindow().queue([shared = shared_from_this()] {
				shared->reuploadFluids();
				shared->fluidReuploadPending = false;
			});
		}
	}

	void Realm::autotile(const Position &position, Layer layer) {
		const Tileset &tileset = getTileset();
		const TileID tile = tileProvider.copyTile(layer, position, TileProvider::TileMode::ReturnEmpty);
		const auto &tilename = tileset[tile];

		if (const MarchableInfo *info = tileset.getMarchableInfo(tilename)) {
			const auto &members = info->autotileSet->members;

			const TileID march_result = march4([&](int8_t march_row_offset, int8_t march_column_offset) -> bool {
				const Position march_position = position + Position(march_row_offset, march_column_offset);
				const TileID march_tile = tileProvider.copyTile(layer, march_position, TileProvider::TileMode::ReturnEmpty);
				return members.contains(tileset[march_tile]);
			});

			const TileID marched = tileset[info->start] + march_result;

			if (marched != tile) {
				setTile(layer, position, marched, true);
				threadContext.updatedLayers.insert(layer);
			}
		}
	}

	bool Realm::rightClick(const Position &position, double x, double y) {
		if (getSide() != Side::Client)
			return false;

		auto &game = getGame().toClient();
		const auto player     = game.player;
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
					if (overlap)
						agent->onInteractOn(player, Modifiers{});
					else
						agent->onInteractNextTo(player, Modifiers{});
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
			auto &window = game.getWindow();
			auto &menu = window.glMenu;
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

	void Realm::initEntity(const EntityPtr &entity, const Position &position) {
		entity->init(getGame());
		entity->spawning = false;
		add(entity, position);
		entity->calculateVisibleEntities();

		if (getSide() == Side::Server) {
			auto lock = entity->visiblePlayers.sharedLock();
			if (!entity->visiblePlayers.empty()) {
				const EntityPacket packet(entity);
				for (const auto &weak_player: entity->visiblePlayers) {
					if (auto player = weak_player.lock()) {
						INFO("Notifying " << player->username << " of " << entity->getName() << " " << entity->getGID());
						player->notifyOfRealm(*this);
						player->send(packet);
					}
				}
			} else {
				WARN("Visible players set is empty");
			}
		}
	}

	BiomeType Realm::getBiome(int64_t seed) {
		std::default_random_engine rng{seed * 79ul};
		return std::uniform_int_distribution(0, 100)(rng) % Biome::COUNT + 1;
	}
}
