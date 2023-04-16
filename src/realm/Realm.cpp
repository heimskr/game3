#include <iostream>
#include <thread>
#include <unordered_set>

#include "MarchingSquares.h"
#include "Tileset.h"
#include "biome/Biome.h"
#include "entity/Entity.h"
#include "game/Game.h"
#include "game/InteractionSet.h"
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

	Realm::Realm(Game &game_): game(game_) {}

	Realm::Realm(Game &game_, RealmID id_, RealmType type_, TilemapPtr tilemap1_, TilemapPtr tilemap2_, TilemapPtr tilemap3_, BiomeMapPtr biome_map, int seed_):
	id(id_), type(type_), tilemap1(std::move(tilemap1_)), tilemap2(std::move(tilemap2_)), tilemap3(std::move(tilemap3_)), biomeMap(std::move(biome_map)), seed(seed_), game(game_) {
		tilemap1->init(game);
		tilemap2->init(game);
		tilemap3->init(game);
		renderer1.init(tilemap1);
		renderer2.init(tilemap2);
		renderer3.init(tilemap3);
		initTexture();
		resetPathMap();
	}

	Realm::Realm(Game &game_, RealmID id_, RealmType type_, TilemapPtr tilemap1_, BiomeMapPtr biome_map, int seed_):
	id(id_), type(type_), tilemap1(std::move(tilemap1_)), biomeMap(std::move(biome_map)), seed(seed_), game(game_) {
		tilemap1->init(game);
		renderer1.init(tilemap1);
		tilemap2 = std::make_shared<Tilemap>(tilemap1->width, tilemap1->height, tilemap1->tileSize, tilemap1->tileset);
		tilemap3 = std::make_shared<Tilemap>(tilemap1->width, tilemap1->height, tilemap1->tileSize, tilemap1->tileset);
		initTexture();
		tilemap2->init(game);
		tilemap3->init(game);
		renderer2.init(tilemap2);
		renderer3.init(tilemap3);
		resetPathMap();
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
		tilemap1 = std::make_shared<Tilemap>(Tilemap::fromJSON(game, json.at("tilemap1")));
		tilemap2 = std::make_shared<Tilemap>(Tilemap::fromJSON(game, json.at("tilemap2")));
		tilemap3 = std::make_shared<Tilemap>(Tilemap::fromJSON(game, json.at("tilemap3")));
		tilemap1->init(game);
		tilemap2->init(game);
		tilemap3->init(game);
		initTexture();
		biomeMap = std::make_shared<BiomeMap>(json.at("biomeMap"));
		outdoors = json.at("outdoors");
		for (const auto &[index, tile_entity_json]: json.at("tileEntities").get<std::unordered_map<std::string, nlohmann::json>>()) {
			auto tile_entity = TileEntity::fromJSON(game, tile_entity_json);
			tileEntities.emplace(parseUlong(index), tile_entity);
			tile_entity->setRealm(shared);
			tile_entity->onSpawn();
			if (tile_entity_json.at("id").get<Identifier>() == "base:te/ghost"_id)
				++ghostCount;
		}
		renderer1.init(tilemap1);
		renderer2.init(tilemap2);
		renderer3.init(tilemap3);
		entities.clear();
		for (const auto &entity_json: json.at("entities"))
			(*entities.insert(Entity::fromJSON(game, entity_json)).first)->setRealm(shared);
		if (json.contains("extra"))
			extraData = json.at("extra");
	}

	void Realm::render(const int width, const int height, const Eigen::Vector2f &center, float scale, SpriteRenderer &sprite_renderer, float game_time) {
		Canvas &canvas = game.canvas;
		auto &textureA = canvas.textureA;
		auto &textureB = canvas.textureB;
		// auto &fbo = canvas.fbo;
		auto &multiplier = canvas.multiplier;

		if (canvas.lastRealm != this) {
			constexpr float factor = ElementBufferedRenderer::TEXTURE_SCALE;
			const auto texture_width  = tilemap1->width  * tilemap1->tileSize * factor;
			const auto texture_height = tilemap1->height * tilemap1->tileSize * factor;
			if (textureA.getWidth() != texture_width || textureA.getHeight() != texture_height) {
				textureA.initFloat(texture_width, texture_height, GL_NEAREST);
				textureB.initFloat(texture_width, texture_height, GL_NEAREST);
			}
			canvas.lastRealm = this;
		}

		// const auto bb_width  = textureA.getWidth();
		// const auto bb_height = textureA.getHeight();
		// fbo.bind();
		// textureA.useInFB();
		// GL::Viewport viewport(0, 0, bb_width, bb_height);
		const auto bb_width  = width;
		const auto bb_height = height;
		renderer1.onBackbufferResized(bb_width, bb_height);
		renderer2.onBackbufferResized(bb_width, bb_height);
		renderer3.onBackbufferResized(bb_width, bb_height);
		renderer1.render(outdoors? game_time : 1, scale, center.x(), center.y());
		renderer2.render(outdoors? game_time : 1, scale, center.x(), center.y());
		renderer3.render(outdoors? game_time : 1, scale, center.x(), center.y());
		sprite_renderer.update(bb_width, bb_height);

		std::shared_ptr<Entity> player;
		for (const auto &entity: entities)
			if (entity->isPlayer())
				player = entity;
			else
				entity->render(sprite_renderer);
		for (const auto &[index, tile_entity]: tileEntities)
			tile_entity->render(sprite_renderer);

		if (player)
			player->render(sprite_renderer);

		multiplier.update(bb_width, bb_height);
		// sprite_renderer.drawOnMap(texture, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f, 1.f);
		// textureB.useInFB();
		// game.canvas.multiplier(textureA, renderer1.lightTexture);
		// textureA.useInFB();
		// game.canvas.multiplier(textureB, renderer2.lightTexture);
		// textureB.useInFB();
		// game.canvas.multiplier(textureA, renderer3.lightTexture);
		// sprite_renderer.drawOnScreen(renderer1.lightTexture, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f);
		// sprite_renderer.drawOnScreen(renderer2.lightTexture, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f);
		// sprite_renderer.drawOnScreen(renderer3.lightTexture, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f);

		// fbo.undo();
		// viewport.reset();

		// sprite_renderer.update(width, height);
		// sprite_renderer.drawOnMap(textureB, 0.f, 0.f, 0.f, 0.f, -1.f, -1.f, 1.f);
		// sprite_renderer.drawOnMap(textureA, {
		// 	.size_x = -1.f,
		// 	.size_y = -1.f,
		// });

		if (0 < ghostCount) {
			static auto checkmark = cacheTexture("resources/checkmark.png");
			sprite_renderer.drawOnScreen(*checkmark, {
				.x = static_cast<float>(width)  / *checkmark->width  - 3.f,
				.y = static_cast<float>(height) / *checkmark->height - 3.f,
				.scale = 2.f,
				.hackY = false,
				.invertY = false,
			});
		}
	}

	void Realm::reupload() {
		renderer1.reupload();
		renderer2.reupload();
		renderer3.reupload();
}

	void Realm::rebind() {
		renderer1.tilemap = tilemap1;
		renderer2.tilemap = tilemap2;
		renderer3.tilemap = tilemap3;
	}

	EntityPtr Realm::add(const EntityPtr &entity) {
		entity->setRealm(shared_from_this());
		entities.insert(entity);
		return entity;
	}

	TileEntityPtr Realm::add(const TileEntityPtr &tile_entity) {
		const Index index = getIndex(tile_entity->position);
		if (tileEntities.contains(index))
			return nullptr;
		tile_entity->setRealm(shared_from_this());
		tileEntities.emplace(index, tile_entity);
		if (tile_entity->solid)
			pathMap[index] = false;
		if (tile_entity->is("base:te/ghost"))
			++ghostCount;
		tile_entity->onSpawn();
		return tile_entity;
	}

	TileEntityPtr Realm::addSafe(const TileEntityPtr &tile_entity) {
		auto lock = tileEntityLock.lockWrite(std::chrono::milliseconds(1));
		return add(tile_entity);
	}

	void Realm::initEntities() {
		for (auto &entity: entities)
			entity->setRealm(shared_from_this());
	}

	void Realm::tick(float delta) {
		ticking = true;
		for (auto &entity: entities)
			if (entity->isPlayer()) {
				auto player = std::dynamic_pointer_cast<Player>(entity);
				if (!player->ticked) {
					player->ticked = true;
					player->tick(game, delta);
				}
			} else
				entity->tick(game, delta);
		for (auto &[index, tile_entity]: tileEntities)
			tile_entity->tick(game, delta);
		ticking = false;
		for (const auto &entity: entityRemovalQueue)
			remove(entity);
		entityRemovalQueue.clear();
		for (const auto &tile_entity: tileEntityRemovalQueue)
			remove(tile_entity);
		tileEntityRemovalQueue.clear();
	}

	std::vector<EntityPtr> Realm::findEntities(const Position &position) const {
		std::vector<EntityPtr> out;
		for (const auto &entity: entities)
			if (entity->position == position)
				out.push_back(entity);
		return out;
	}

	std::vector<EntityPtr> Realm::findEntities(const Position &position, const EntityPtr &except) const {
		std::vector<EntityPtr> out;
		for (const auto &entity: entities)
			if (entity->position == position && entity != except)
				out.push_back(entity);
		return out;
	}

	EntityPtr Realm::findEntity(const Position &position) const {
		for (const auto &entity: entities)
			if (entity->position == position)
				return entity;
		return {};
	}

	EntityPtr Realm::findEntity(const Position &position, const EntityPtr &except) const {
		for (const auto &entity: entities)
			if (entity->position == position && entity != except)
				return entity;
		return {};
	}

	TileEntityPtr Realm::tileEntityAt(const Position &position) {
		auto lock = tileEntityLock.lockRead();
		const auto iter = tileEntities.find(getIndex(position));
		if (iter == tileEntities.end())
			return {};
		return iter->second;
	}

	void Realm::remove(EntityPtr entity) {
		entities.erase(entity);
	}

	void Realm::remove(const TileEntityPtr &tile_entity, bool run_helper) {
		const Position position = tile_entity->position;
		const Index index = getIndex(position);
		tileEntities.at(index)->onRemove();
		tileEntities.erase(index);
		if (run_helper)
			setLayerHelper(index, false);
		if (tile_entity->is("base:te/ghost"))
			--ghostCount;
		updateNeighbors(position);
	}

	void Realm::removeSafe(const TileEntityPtr &tile_entity) {
		auto lock = tileEntityLock.lockWrite(std::chrono::milliseconds(1));
		remove(tile_entity, false);
	}

	Position Realm::getPosition(Index index) const {
		return {index / getWidth(), index % getWidth()};
	}

	void Realm::onMoved(const EntityPtr &entity, const Position &position) {
		if (auto tile_entity = tileEntityAt(position))
			tile_entity->onOverlap(entity);
	}

	Game & Realm::getGame() {
		return game;
	}

	void Realm::queueRemoval(const EntityPtr &entity) {
		if (ticking)
			entityRemovalQueue.push_back(entity);
		else
			remove(entity);
	}

	void Realm::queueRemoval(const TileEntityPtr &tile_entity) {
		if (ticking)
			tileEntityRemovalQueue.push_back(tile_entity);
		else
			remove(tile_entity);
	}

	void Realm::absorb(const EntityPtr &entity, const Position &position) {
		if (auto realm = entity->weakRealm.lock())
			realm->remove(entity);
		entity->setRealm(shared_from_this());
		entity->init(getGame());
		entity->teleport(position);
	}

	void Realm::setLayer1(Index row, Index column, TileID tile, bool run_helper) {
		tilemap1->set(column, row, tile);
		if (run_helper)
			setLayerHelper(row, column);
	}

	void Realm::setLayer2(Index row, Index column, TileID tile, bool run_helper) {
		tilemap2->set(column, row, tile);
		if (run_helper)
			setLayerHelper(row, column);
	}

	void Realm::setLayer3(Index row, Index column, TileID tile, bool run_helper) {
		tilemap3->set(column, row, tile);
		if (run_helper)
			setLayerHelper(row, column);
	}

	void Realm::setLayer1(Index index, TileID tile, bool run_helper) {
		tilemap1->set(index, tile);
		if (run_helper)
			setLayerHelper(index);
	}

	void Realm::setLayer2(Index index, TileID tile, bool run_helper) {
		tilemap2->set(index, tile);
		if (run_helper)
			setLayerHelper(index);
	}

	void Realm::setLayer3(Index index, TileID tile, bool run_helper) {
		tilemap3->set(index, tile);
		if (run_helper)
			setLayerHelper(index);
	}

	void Realm::setLayer1(Index index, const Identifier &tilename, bool run_helper) {
		tilemap1->set(index, (*tilemap1->tileset)[tilename]);
		if (run_helper)
			setLayerHelper(index);
	}

	void Realm::setLayer2(Index index, const Identifier &tilename, bool run_helper) {
		tilemap2->set(index, (*tilemap2->tileset)[tilename]);
		if (run_helper)
			setLayerHelper(index);
	}

	void Realm::setLayer3(Index index, const Identifier &tilename, bool run_helper) {
		tilemap3->set(index, (*tilemap3->tileset)[tilename]);
		if (run_helper)
			setLayerHelper(index);
	}

	void Realm::setLayer1(const Position &position, TileID tile, bool run_helper) {
		setLayer1(position.row, position.column, tile, run_helper);
	}

	void Realm::setLayer2(const Position &position, TileID tile, bool run_helper) {
		setLayer2(position.row, position.column, tile, run_helper);
	}

	void Realm::setLayer3(const Position &position, TileID tile, bool run_helper) {
		setLayer3(position.row, position.column, tile, run_helper);
	}

	void Realm::setLayer1(const Position &position, const Identifier &tile, bool run_helper) {
		setLayer1(position.row, position.column, (*tilemap1->tileset)[tile], run_helper);
	}

	void Realm::setLayer2(const Position &position, const Identifier &tile, bool run_helper) {
		setLayer2(position.row, position.column, (*tilemap2->tileset)[tile], run_helper);
	}

	void Realm::setLayer3(const Position &position, const Identifier &tile, bool run_helper) {
		setLayer3(position.row, position.column, (*tilemap3->tileset)[tile], run_helper);
	}

	TileID Realm::getLayer1(Index row, Index column) const {
		return (*tilemap1)(column, row);
	}

	TileID Realm::getLayer2(Index row, Index column) const {
		return (*tilemap2)(column, row);
	}

	TileID Realm::getLayer3(Index row, Index column) const {
		return (*tilemap3)(column, row);
	}

	TileID Realm::getLayer1(Index index) const {
		return (*tilemap1)[index];
	}

	TileID Realm::getLayer2(Index index) const {
		return (*tilemap2)[index];
	}

	TileID Realm::getLayer3(Index index) const {
		return (*tilemap3)[index];
	}

	TileID Realm::getLayer1(const Position &position) const {
		return (*tilemap1)[position];
	}

	TileID Realm::getLayer2(const Position &position) const {
		return (*tilemap2)[position];
	}

	TileID Realm::getLayer3(const Position &position) const {
		return (*tilemap3)[position];
	}

	bool Realm::interactGround(const PlayerPtr &player, const Position &position) {
		if (!isValid(position))
			return false;

		const Place place(position, shared_from_this(), player);
		auto &game = getGame();

		if (auto iter = game.interactionSets.find(type); iter != game.interactionSets.end())
			if (iter->second->interact(place))
				return true;

		return false;
	}

	std::optional<Position> Realm::getPathableAdjacent(const Position &position) const {
		const auto width  = getWidth();
		const auto height = getHeight();

		if (position.row < height - 1) {
			const Position next(position.row + 1, position.column);
			if (pathMap[getIndex(next)])
				return next;
		}

		if (position.column < width - 1) {
			const Position next(position.row, position.column + 1);
			if (pathMap[getIndex(next)])
				return next;
		}

		if (0 < position.row) {
			const Position next(position.row - 1, position.column);
			if (pathMap[getIndex(next)])
				return next;
		}

		if (0 < position.column) {
			const Position next(position.row, position.column - 1);
			if (pathMap[getIndex(next)])
				return next;
		}

		return std::nullopt;
	}

	std::optional<Position> Realm::getPathableAdjacent(Index index) const {
		return getPathableAdjacent(getPosition(index));
	}

	bool Realm::isValid(const Position &position) const {
		return 0 <= position.row && position.row < getHeight() && 0 <= position.column && position.column < getWidth();
	}

	void Realm::updateNeighbors(const Position &position) {
		static size_t depth = 0;
		static bool layer2_updated = false;

		++depth;

		const auto &tiles = tilemap2->getTiles();
		const auto &tileset = *tilemap2->tileset;

		for (Index row_offset = -1; row_offset <= 1; ++row_offset)
			for (Index column_offset = -1; column_offset <= 1; ++column_offset)
				if (row_offset != 0 || column_offset != 0) {
					const Position offset_position = position + Position(row_offset, column_offset);
					if (!isValid(offset_position))
						continue;
					if (auto neighbor = tileEntityAt(offset_position)) {
						neighbor->onNeighborUpdated(-row_offset, -column_offset);
					} else {
						const TileID tile = tiles.at(getIndex(offset_position));
						const auto &tilename = tileset[tile];

						for (const auto &category: tileset.getCategories(tilename)) {
							if (tileset.isCategoryMarchable(category)) {
								TileID march_result = march4([&](int8_t march_row_offset, int8_t march_column_offset) -> bool {
									const Position march_position = offset_position + Position(march_row_offset, march_column_offset);
									if (!isValid(march_position))
										return false;
									return tileset.isInCategory(tileset[tiles.at(getIndex(march_position))], category);
								});

								// ???
								const TileID marched = (march_result / 7 + 6) * (tilemap2->setWidth / tilemap2->tileSize) + march_result % 7;
								if (marched != tile) {
									tilemap2->set(offset_position, marched);
									layer2_updated = true;
								}
							}
						}
					}
				}

		if (--depth == 0 && layer2_updated) {
			layer2_updated = false;
			getGame().activateContext();
			renderer2.reupload();
		}
	}

	bool Realm::hasTileEntityAt(const Position &position) const {
		return tileEntities.contains(getIndex(position));
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

		game.activateContext();
		renderer2.reupload();
	}

	void Realm::damageGround(const Position &position) {
		const Place place(position, shared_from_this(), nullptr);
		auto &game = getGame();

		if (auto iter = game.interactionSets.find(type); iter != game.interactionSets.end())
			iter->second->damageGround(place);
	}

	const Tileset & Realm::getTileset() const {
		return *tilemap1->tileset;
	}

	void Realm::toJSON(nlohmann::json &json) const {
		json["id"] = id;
		json["type"] = type;
		json["seed"] = seed;
		json["tilemap1"] = *tilemap1;
		json["tilemap2"] = *tilemap2;
		json["tilemap3"] = *tilemap3;
		json["biomeMap"] = *biomeMap;
		json["outdoors"] = outdoors;
		json["tileEntities"] = std::unordered_map<std::string, nlohmann::json>();
		for (const auto &[index, tile_entity]: tileEntities)
			json["tileEntities"][std::to_string(index)] = *tile_entity;
		json["entities"] = std::vector<nlohmann::json>();
		for (const auto &entity: entities) {
			nlohmann::json entity_json;
			entity->toJSON(entity_json);
			json["entities"].push_back(std::move(entity_json));
		}
		if (!extraData.empty())
			json["extra"] = extraData;
	}

	bool Realm::isWalkable(Index row, Index column, const Tileset &tileset) const {
		if (!tileset.isWalkable((*tilemap1)(column, row)) || !tileset.isWalkable((*tilemap2)(column, row)) || !tileset.isWalkable((*tilemap3)(column, row)))
			return false;
		const Index index = getIndex(row, column);
		if (tileEntities.contains(index) && tileEntities.at(index)->solid)
			return false;
		return true;
	}

	void Realm::setLayerHelper(Index row, Index column, bool should_mark_dirty) {
		const auto &tileset = getTileset();
		const Position position(row, column);
		pathMap[getIndex(position)] = isWalkable(row, column, tileset);
		updateNeighbors(position);
		if (should_mark_dirty) {
			renderer1.markDirty();
			renderer2.markDirty();
			renderer3.markDirty();
		}
	}

	void Realm::setLayerHelper(Index index, bool should_mark_dirty) {
		const auto &tileset = getTileset();
		const Position position = getPosition(index);
		pathMap[index] = isWalkable(position.row, position.column, tileset);
		updateNeighbors(position);
		if (should_mark_dirty) {
			renderer1.markDirty();
			renderer2.markDirty();
			renderer3.markDirty();
		}
	}

	void Realm::resetPathMap() {
		const auto width = tilemap1->width;
		const auto height = tilemap1->height;
		pathMap.resize(width * height);
		const auto &tileset = getTileset();
		for (Index row = 0; row < height; ++row)
			for (Index column = 0; column < width; ++column)
				pathMap[getIndex(row, column)] = isWalkable(row, column, tileset);
	}

	bool Realm::rightClick(const Position &position, double x, double y) {
		auto entities = findEntities(position);

		const auto player = getGame().player;
		const auto player_pos = player->getPosition();
		const bool overlap = player_pos == position;
		const bool adjacent = position.adjacent4(player_pos);
		if (!overlap && !adjacent)
			return false;

		if (!entities.empty()) {
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

			auto &window = getGame().getWindow();
			auto &menu = window.glMenu;
			window.remove_action_group("entity_menu");
			window.insert_action_group("entity_menu", group);
			menu.set_menu_model(gmenu);
			menu.set_has_arrow(true);
			std::cerr << "(" << x << ", " << y << ") -> (" << int(x) << ", " << int(y) << ")\n";
			menu.set_pointing_to({int(x), int(y), 1, 1});
			menu.popup();
			return true;
		}

		return false;
	}

	BiomeType Realm::getBiome(uint32_t seed) {
		std::default_random_engine rng;
		rng.seed(seed * 79);
		return std::uniform_int_distribution(0, 100)(rng) % Biome::COUNT + 1;
	}

	void to_json(nlohmann::json &json, const Realm &realm) {
		realm.toJSON(json);
	}
}
