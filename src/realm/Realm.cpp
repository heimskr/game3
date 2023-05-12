#include <iostream>
#include <thread>
#include <unordered_set>

#include "MarchingSquares.h"
#include "ThreadContext.h"
#include "Tileset.h"
#include "biome/Biome.h"
#include "entity/Entity.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "game/InteractionSet.h"
#include "game/ServerGame.h"
#include "realm/Keep.h"
#include "realm/Realm.h"
#include "realm/RealmFactory.h"
#include "tileentity/Ghost.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Carpet.h"
#include "worldgen/House.h"
#include "worldgen/Keep.h"

namespace Game3 {
	void from_json(const nlohmann::json &json, RealmDetails &details) {
		details.tilesetName = json.at("tileset");
	}

	Realm::Realm(Game &game_): game(game_) {
		if (game.getSide() == Side::Client) {
			createRenderers();
			initRendererRealms();
			initRendererTileProviders();
		}
	}

	Realm::Realm(Game &game_, RealmID id_, RealmType type_, Identifier tileset_id, int64_t seed_):
	id(id_), type(type_), tileProvider(std::move(tileset_id)), seed(seed_), game(game_) {
		if (game.getSide() == Side::Client) {
			createRenderers();
			initRendererRealms();
			initTexture();
			initRendererTileProviders();
		}
	}

	void Realm::initRendererRealms() {
		for (auto &row: *renderers)
			for (auto &layers: row)
				for (auto &renderer: layers)
					renderer.setRealm(*this);
	}

	void Realm::initRendererTileProviders() {
		for (auto &row: *renderers) {
			for (auto &layers: row) {
				Layer layer = 0;
				for (auto &renderer: layers)
					renderer.setup(tileProvider, ++layer);
			}
		}
	}

	void Realm::initTexture() {}

	RealmPtr Realm::fromJSON(Game &game, const nlohmann::json &json) {
		const RealmType type = json.at("type");
		auto factory = game.registry<RealmFactoryRegistry>().at(type);
		assert(factory);
		auto out = (*factory)(game);
		out->absorbJSON(json);
		return out;
	}

	void Realm::absorbJSON(const nlohmann::json &json) {
		auto shared = shared_from_this();
		id = json.at("id");
		type = json.at("type");
		seed = json.at("seed");
		generatedChunks = json.at("generatedChunks");
		tileProvider.clear();
		from_json(json.at("tilemap"), tileProvider);
		initRendererTileProviders();
		initTexture();
		outdoors = json.at("outdoors");
		for (const auto &[position_string, tile_entity_json]: json.at("tileEntities").get<std::unordered_map<std::string, nlohmann::json>>()) {
			auto tile_entity = TileEntity::fromJSON(game, tile_entity_json);
			tileEntities.emplace(Position(position_string), tile_entity);
			tileEntitiesByGID.emplace(tile_entity->globalID, tile_entity);
			tile_entity->setRealm(shared);
			tile_entity->onSpawn();
			if (tile_entity_json.at("id").get<Identifier>() == "base:te/ghost"_id)
				++ghostCount;
		}
		entities.clear();
		for (const auto &entity_json: json.at("entities"))
			(*entities.insert(Entity::fromJSON(game, entity_json)).first)->setRealm(shared);
		if (json.contains("extra"))
			extraData = json.at("extra");
	}

	void Realm::onFocus() {
		if (getSide() != Side::Client)
			return;

		focused = true;
		for (auto &row: *renderers)
			for (auto &layers: row)
				for (auto &renderer: layers)
					renderer.wakeUp();
		reupload();
	}

	void Realm::onBlur() {
		if (getSide() != Side::Client)
			return;

		focused = false;
		for (auto &row: *renderers)
			for (auto &layers: row)
				for (auto &renderer: layers)
					renderer.snooze();
	}

	void Realm::createRenderers() {
		if (getSide() != Side::Client)
			return;

		renderers.emplace();
	}

	void Realm::render(const int width, const int height, const Eigen::Vector2f &center, float scale, SpriteRenderer &sprite_renderer, float game_time) {
		if (getSide() != Side::Client)
			return;

		if (!focused)
			onFocus();

		Canvas &canvas = game.toClient().canvas;
		auto &multiplier = canvas.multiplier;

		const auto bb_width  = width;
		const auto bb_height = height;

		for (auto &row: *renderers) {
			for (auto &layers: row) {
				for (auto &renderer: layers) {
					renderer.onBackbufferResized(bb_width, bb_height);
					renderer.render(outdoors? game_time : 1, scale, center.x(), center.y());
				}
			}
		}

		sprite_renderer.centerX = center.x();
		sprite_renderer.centerY = center.y();
		sprite_renderer.update(bb_width, bb_height);
		sprite_renderer.divisor = outdoors? game_time : 1;

		std::shared_ptr<Entity> player;
		for (const auto &entity: entities)
			if (entity->isPlayer()) {
				player = entity;
				const auto [x, y] = getChunkPosition(player->getPosition());
				for (int32_t x_offset = -REALM_DIAMETER / 2; x_offset <= REALM_DIAMETER / 2; ++x_offset) {
					for (int32_t y_offset = -REALM_DIAMETER / 2; y_offset <= REALM_DIAMETER / 2; ++y_offset) {
						ChunkPosition chunk_position{x + x_offset, y + y_offset};
						if (!generatedChunks.contains(chunk_position))
							tileProvider.generationQueue.push_back(chunk_position);
					}
				}
			} else
				entity->render(sprite_renderer);
		for (const auto &[index, tile_entity]: tileEntities)
			tile_entity->render(sprite_renderer);

		if (player)
			player->render(sprite_renderer);

		multiplier.update(bb_width, bb_height);
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
		// 	.size_x = -1.f,
		// 	.size_y = -1.f,
		// });

		if (0 < ghostCount) {
			static auto checkmark = cacheTexture("resources/checkmark.png");
			sprite_renderer.drawOnScreen(*checkmark, {
				.x = static_cast<float>(width)  / *checkmark->width  - 3.f,
				.y = static_cast<float>(height) / *checkmark->height - 3.f,
				.scaleX = 2.f,
				.scaleY = 2.f,
				.hackY = false,
				.invertY = false,
			});
		}
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
				layers[layer - 1].reupload();
	}

	EntityPtr Realm::addUnsafe(const EntityPtr &entity) {
		entity->setRealm(shared_from_this());
		entities.insert(entity);
		if (entity->isPlayer())
			players.insert(std::dynamic_pointer_cast<Player>(entity));
		return entity;
	}

	EntityPtr Realm::add(const EntityPtr &entity) {
		auto lock = lockEntitiesUnique();
		return addUnsafe(entity);
	}

	TileEntityPtr Realm::addUnsafe(const TileEntityPtr &tile_entity) {
		if (tileEntities.contains(tile_entity->position))
			return nullptr;
		tile_entity->setRealm(shared_from_this());
		tileEntities.emplace(tile_entity->position, tile_entity);
		tileEntitiesByGID.emplace(tile_entity->globalID, tile_entity);
		if (tile_entity->solid)
			tileProvider.findPathState(tile_entity->position) = false;
		if (tile_entity->is("base:te/ghost"))
			++ghostCount;
		tile_entity->onSpawn();
		return tile_entity;
	}

	TileEntityPtr Realm::add(const TileEntityPtr &tile_entity) {
		auto lock = lockTileEntitiesUnique();
		return addUnsafe(tile_entity);
	}

	void Realm::initEntities() {
		for (auto &entity: entities) {
			entity->setRealm(shared_from_this());
			if (auto player = std::dynamic_pointer_cast<Player>(entity))
				players.insert(player);
		}
	}

	void Realm::tick(float delta) {
		ChunkPosition player_cpos {};

		ticking = true;

		for (const auto &stolen: entityAdditionQueue.steal())
			add(stolen);

		for (const auto &stolen: tileEntityAdditionQueue.steal())
			add(stolen);

		for (auto &entity: entities) {
			if (entity->isPlayer()) {
				auto player = std::dynamic_pointer_cast<Player>(entity);
				player_cpos = getChunkPosition(player->getPosition());
				if (!player->ticked) {
					player->ticked = true;
					player->tick(game, delta);
				}
			} else
				entity->tick(game, delta);
		}

		for (auto &[index, tile_entity]: tileEntities)
			tile_entity->tick(game, delta);

		ticking = false;

		for (const auto &stolen: entityRemovalQueue.steal())
			remove(stolen);

		for (const auto &stolen: tileEntityRemovalQueue.steal())
			remove(stolen);

		for (const auto &stolen: generalQueue.steal())
			stolen();

		if (!tileProvider.generationQueue.empty()) {
			const auto chunk_position = std::move(tileProvider.generationQueue.back());
			tileProvider.generationQueue.pop_back();
			if (!generatedChunks.contains(chunk_position)) {
				generateChunk(chunk_position);
				generatedChunks.insert(chunk_position);
				remakePathMap(chunk_position);
				reupload();
			}
		}

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
	}

	std::vector<EntityPtr> Realm::findEntities(const Position &position) {
		auto lock = lockEntitiesShared();
		std::vector<EntityPtr> out;
		for (const auto &entity: entities)
			if (entity->position == position)
				out.push_back(entity);
		return out;
	}

	std::vector<EntityPtr> Realm::findEntities(const Position &position, const EntityPtr &except) {
		std::vector<EntityPtr> out;
		auto lock = lockEntitiesShared();
		for (const auto &entity: entities)
			if (entity->position == position && entity != except)
				out.push_back(entity);
		return out;
	}

	EntityPtr Realm::findEntity(const Position &position) {
		auto lock = lockEntitiesShared();
		for (const auto &entity: entities)
			if (entity->position == position)
				return entity;
		return {};
	}

	EntityPtr Realm::findEntity(const Position &position, const EntityPtr &except) {
		auto lock = lockEntitiesShared();
		for (const auto &entity: entities)
			if (entity->position == position && entity != except)
				return entity;
		return {};
	}

	TileEntityPtr Realm::tileEntityAt(const Position &position) {
		auto lock = lockTileEntitiesShared();
		if (auto iter = tileEntities.find(position); iter != tileEntities.end())
			return iter->second;
		return {};
	}

	void Realm::remove(const EntityPtr &entity) {
		entities.erase(entity);
		if (auto player = std::dynamic_pointer_cast<Player>(entity))
			entities.erase(player);
	}

	void Realm::removeSafe(const EntityPtr &entity) {
		auto lock = lockEntitiesUnique();
		remove(entity);
	}

	void Realm::remove(TileEntityPtr tile_entity, bool run_helper) {
		const Position position = tile_entity->position;
		tileEntities.at(position)->onRemove();
		tileEntities.erase(position);
		tileEntitiesByGID.erase(tile_entity->globalID);
		if (run_helper)
			setLayerHelper(position.row, position.column, false);
		if (tile_entity->is("base:te/ghost"))
			--ghostCount;
		updateNeighbors(position);
	}

	void Realm::removeSafe(const TileEntityPtr &tile_entity) {
		auto lock = lockTileEntitiesUnique();
		remove(tile_entity, false);
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

	void Realm::queueAddition(const EntityPtr &entity) {
		entityAdditionQueue.push(entity);
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
		tileProvider.findTile(layer, row, column, TileProvider::TileMode::Create) = tile_id;
		if (run_helper)
			setLayerHelper(row, column);
	}

	void Realm::setTile(Layer layer, const Position &position, TileID tile_id, bool run_helper) {
		tileProvider.findTile(layer, position.row, position.column, TileProvider::TileMode::Create) = tile_id;
		if (isServer()) {
			if (run_helper)
				setLayerHelper(position.row, position.column);
			getGame().toServer().broadcastTileUpdate(id, layer, position, tile_id);
		}
	}

	void Realm::setTile(Layer layer, const Position &position, const Identifier &tilename, bool run_helper) {
		setTile(layer, position, getTileset()[tilename], run_helper);
	}

	TileID Realm::getTile(Layer layer, Index row, Index column) const {
		return tileProvider.copyTile(layer, row, column, TileProvider::TileMode::Throw);
	}

	TileID Realm::getTile(Layer layer, const Position &position) const {
		return getTile(layer, position.row, position.column);
	}

	std::optional<TileID> Realm::tryTile(Layer layer, const Position &position) const {
		return tileProvider.tryTile(layer, position);
	}

	bool Realm::interactGround(const PlayerPtr &player, const Position &position, Modifiers modifiers) {
		const Place place(position, shared_from_this(), player);
		auto &game = getGame();

		if (auto iter = game.interactionSets.find(type); iter != game.interactionSets.end())
			if (iter->second->interact(place, modifiers))
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

	void Realm::updateNeighbors(const Position &position) {
		if (updatesPaused)
			return;

		++threadContext.updateNeighborsDepth;
		auto &tileset = getTileset();

		for (Index row_offset = -1; row_offset <= 1; ++row_offset) {
			for (Index column_offset = -1; column_offset <= 1; ++column_offset) {
				if (row_offset != 0 || column_offset != 0) {
					const Position offset_position = position + Position(row_offset, column_offset);
					if (auto neighbor = tileEntityAt(offset_position)) {
						neighbor->onNeighborUpdated(-row_offset, -column_offset);
					} else {
						const TileID tile = tileProvider.copyTile(2, offset_position, TileProvider::TileMode::ReturnEmpty);
						const auto &tilename = tileset[tile];

						for (const auto &category: tileset.getCategories(tilename)) {
							if (tileset.isCategoryMarchable(category)) {
								TileID march_result = march4([&](int8_t march_row_offset, int8_t march_column_offset) -> bool {
									const Position march_position = offset_position + Position(march_row_offset, march_column_offset);
									return tileset.isInCategory(tileset[tileProvider.copyTile(2, march_position, TileProvider::TileMode::ReturnEmpty)], category);
								});

								const TileID marched = tileset[tileset.getMarchBase(category)] + (march_result / 7) * tileset.columnCount(getGame()) + march_result % 7;
								if (marched != tile) {
									setTile(2, offset_position, marched);
									threadContext.layer2Updated = true;
								}
							}
						}
					}
				}
			}
		}

		if (--threadContext.updateNeighborsDepth == 0 && threadContext.layer2Updated) {
			threadContext.layer2Updated = false;
			reupload(2);
		}
	}

	bool Realm::hasTileEntityAt(const Position &position) const {
		return tileEntities.contains(position);
	}

	void Realm::confirmGhosts() {
		if (ghostCount <= 0)
			return;

		std::vector<std::shared_ptr<Ghost>> ghosts;

		for (auto &[index, tile_entity]: tileEntities)
			if (tile_entity->is("base:te/ghost"))
				ghosts.push_back(std::dynamic_pointer_cast<Ghost>(tile_entity));

		for (const auto &ghost: ghosts) {
			remove(ghost);
			ghost->confirm();
		}

		if (getSide() == Side::Client) {
			game.toClient().activateContext();
			reupload(2);
		}
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

	void Realm::toJSON(nlohmann::json &json) const {
		json["id"] = id;
		json["type"] = type;
		json["seed"] = seed;
		json["provider"] = tileProvider;
		json["outdoors"] = outdoors;
		json["generatedChunks"] = generatedChunks;
		json["tileEntities"] = std::unordered_map<std::string, nlohmann::json>();
		json["tilemap"] = tileProvider;
		for (const auto &[position, tile_entity]: tileEntities)
			json["tileEntities"][position.simpleString()] = *tile_entity;
		json["entities"] = std::vector<nlohmann::json>();
		for (const auto &entity: entities) {
			nlohmann::json entity_json;
			entity->toJSON(entity_json);
			json["entities"].push_back(std::move(entity_json));
		}
		if (!extraData.empty())
			json["extra"] = extraData;
	}

	bool Realm::isWalkable(Index row, Index column, const Tileset &tileset) {
		for (Layer layer = 1; layer <= LAYER_COUNT; ++layer)
			if (auto tile = tryTile(layer, {row, column}); !tile || !tileset.isWalkable(*tile))
				return false;
		auto lock = lockTileEntitiesShared();
		if (auto iter = tileEntities.find({row, column}); iter != tileEntities.end() && iter->second->solid)
			return false;
		return true;
	}

	void Realm::setLayerHelper(Index row, Index column, bool should_mark_dirty) {
		const auto &tileset = getTileset();
		const Position position(row, column);
		tileProvider.findPathState(position) = isWalkable(row, column, tileset);

		updateNeighbors(position);
		if (should_mark_dirty)
			for (auto &row: *renderers)
				for (auto &layers: row)
					for (auto &renderer: layers)
						renderer.markDirty();
	}

	void Realm::remakePathMap() {
		const auto &tileset = getTileset();
		for (auto &[chunk_position, path_chunk]: tileProvider.pathMap)
			for (int64_t row = 0; row < CHUNK_SIZE; ++row)
				for (int64_t column = 0; column < CHUNK_SIZE; ++column)
					path_chunk[row * CHUNK_SIZE + column] = isWalkable(row, column, tileset);
	}

	void Realm::remakePathMap(const ChunkRange &range) {
		for (auto y = range.topLeft.y; y <= range.bottomRight.y; ++y)
			for (auto x = range.topLeft.x; x <= range.bottomRight.x; ++x)
				remakePathMap(ChunkPosition{x, y});
	}

	void Realm::remakePathMap(const ChunkPosition &position) {
		const auto &tileset = getTileset();
		auto &path_chunk = tileProvider.getPathChunk(position);
		for (int64_t row = 0; row < CHUNK_SIZE; ++row)
			for (int64_t column = 0; column < CHUNK_SIZE; ++column)
				path_chunk[row * CHUNK_SIZE + column] = isWalkable(position.y * CHUNK_SIZE + row, position.x * CHUNK_SIZE + column, tileset);
	}

	void Realm::markGenerated(const ChunkRange &range) {
		for (auto y = range.topLeft.y; y <= range.bottomRight.y; ++y)
			for (auto x = range.topLeft.x; x <= range.bottomRight.x; ++x)
				generatedChunks.insert(ChunkPosition{x, y});
	}

	void Realm::markGenerated(ChunkPosition chunk_position) {
		generatedChunks.insert(std::move(chunk_position));
	}

	bool Realm::isVisible(const Position &position) const {
		const auto chunk_pos = getChunkPosition(position);

		for (const auto &player: players) {
			const auto player_chunk_pos = getChunkPosition(player->getPosition());
			if (player_chunk_pos.x - REALM_DIAMETER / 2 <= chunk_pos.x && chunk_pos.x <= player_chunk_pos.x + REALM_DIAMETER / 2)
				if (player_chunk_pos.y - REALM_DIAMETER / 2 <= chunk_pos.y && chunk_pos.y <= player_chunk_pos.y + REALM_DIAMETER / 2)
					return true;
		}

		return false;
	}

	bool Realm::hasTileEntity(GlobalID tile_entity_gid) {
		auto lock = lockTileEntitiesShared();
		return tileEntitiesByGID.contains(tile_entity_gid);
	}

	Side Realm::getSide() const {
		return getGame().getSide();
	}

	std::set<ChunkPosition> Realm::getMissingChunks() const {
		assert(getSide() == Side::Client);
		std::set<ChunkPosition> out;
		auto &player = getGame().toClient().player;

		auto chunk_pos = getChunkPosition(player->getPosition());
		chunk_pos.y -= REALM_DIAMETER / 2;
		chunk_pos.x -= REALM_DIAMETER / 2;

		const auto original_x = chunk_pos.x;

		for (const auto &row: *renderers) {
			chunk_pos.x = original_x;

			for (const auto &layers: row) {
				for (const auto &renderer: layers)
					if (renderer.isMissing)
						out.insert(chunk_pos);
				++chunk_pos.x;
			}

			++chunk_pos.y;
		}

		return out;
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

		if (const auto entities = findEntities(position); !entities.empty()) {
			auto gmenu = Gio::Menu::create();
			auto group = Gio::SimpleActionGroup::create();
			size_t i = 0;
			for (const auto &entity: entities) {
				// TODO: Can you escape underscores?
				gmenu->append(entity->getName(), "entity_menu.entity" + std::to_string(i));
				group->add_action("entity" + std::to_string(i++), [entity, overlap, player] {
					if (overlap)
						entity->onInteractOn(player);
					else
						entity->onInteractNextTo(player);
				});
			}

			auto &window = game.getWindow();
			auto &menu = window.glMenu;
			window.remove_action_group("entity_menu");
			window.insert_action_group("entity_menu", group);
			menu.set_menu_model(gmenu);
			menu.set_has_arrow(true);
			menu.set_pointing_to({int(x), int(y), 1, 1});
			menu.popup();
			return true;
		}

		return false;
	}

	BiomeType Realm::getBiome(int64_t seed) {
		std::default_random_engine rng;
		rng.seed(seed * 79);
		return std::uniform_int_distribution(0, 100)(rng) % Biome::COUNT + 1;
	}

	void to_json(nlohmann::json &json, const Realm &realm) {
		realm.toJSON(json);
	}
}
