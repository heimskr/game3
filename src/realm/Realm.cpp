#include <iostream>
#include <thread>
#include <unordered_set>

#include "MarchingSquares.h"
#include "Tiles.h"
#include "entity/Entity.h"
#include "game/Game.h"
#include "game/InteractionSet.h"
#include "realm/Keep.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/Ghost.h"
#include "tileentity/OreDeposit.h"
#include "tileentity/Sign.h"
#include "tileentity/Teleporter.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Carpet.h"
#include "worldgen/House.h"
#include "worldgen/Keep.h"

namespace Game3 {
	std::unordered_map<RealmType, Texture> Realm::textureMap {
		{Realm::OVERWORLD,  cacheTexture("resources/tileset.png")},
		{Realm::HOUSE,      cacheTexture("resources/tileset.png")},
		{Realm::KEEP,       cacheTexture("resources/tileset.png")},
		{Realm::BLACKSMITH, cacheTexture("resources/tileset.png")},
		{Realm::CAVE,       cacheTexture("resources/tileset.png")},
		{Realm::TAVERN,     cacheTexture("resources/tileset.png")},
	};

	std::unordered_set<RealmType> Realm::allTypes {
		Realm::OVERWORLD, Realm::HOUSE, Realm::KEEP, Realm::BLACKSMITH, Realm::CAVE, Realm::TAVERN,
	};

	Realm::Realm(RealmID id_, RealmType type_, const TilemapPtr &tilemap1_, const TilemapPtr &tilemap2_, const TilemapPtr &tilemap3_, int seed_):
	id(id_), type(type_), tilemap1(tilemap1_), tilemap2(tilemap2_), tilemap3(tilemap3_), seed(seed_) {
		tilemap1->init();
		tilemap2->init();
		tilemap3->init();
		renderer1.init(tilemap1);
		renderer2.init(tilemap2);
		renderer3.init(tilemap3);
		resetPathMap();
	}

	Realm::Realm(RealmID id_, RealmType type_, const TilemapPtr &tilemap1_, int seed_): id(id_), type(type_), tilemap1(tilemap1_), seed(seed_) {
		tilemap1->init();
		renderer1.init(tilemap1);
		tilemap2 = std::make_shared<Tilemap>(tilemap1->width, tilemap1->height, tilemap1->tileSize, tilemap1->texture);
		tilemap3 = std::make_shared<Tilemap>(tilemap1->width, tilemap1->height, tilemap1->tileSize, tilemap1->texture);
		tilemap2->init();
		tilemap3->init();
		renderer2.init(tilemap2);
		renderer3.init(tilemap3);
		resetPathMap();
	}

	RealmPtr Realm::fromJSON(const nlohmann::json &json) {
		RealmPtr out;
		const RealmType type = json.at("type");
		switch (type) {
			case Realm::OVERWORLD:
			case Realm::HOUSE:
			case Realm::BLACKSMITH:
			case Realm::CAVE:
			case Realm::TAVERN:
				out = Realm::create();
				break;
			case Realm::KEEP:
				out = Realm::create<Keep>();
				break;
			default:
				throw std::invalid_argument("Invalid realm type: " + std::to_string(int(type)));
		}
		out->absorbJSON(json);
		return out;
	}

	void Realm::absorbJSON(const nlohmann::json &json) {
		auto shared = shared_from_this();
		id = json.at("id");
		type = json.at("type");
		seed = json.at("seed");
		tilemap1 = std::make_shared<Tilemap>(json.at("tilemap1"));
		tilemap2 = std::make_shared<Tilemap>(json.at("tilemap2"));
		tilemap3 = std::make_shared<Tilemap>(json.at("tilemap3"));
		tilemap1->texture.init();
		tilemap2->texture.init();
		tilemap3->texture.init();
		outdoors = json.at("outdoors");
		for (const auto &[index, tile_entity_json]: json.at("tileEntities").get<std::unordered_map<std::string, nlohmann::json>>()) {
			auto tile_entity = TileEntity::fromJSON(tile_entity_json);
			tileEntities.emplace(parseUlong(index), tile_entity);
			tile_entity->setRealm(shared);
			tile_entity->onSpawn();
			if (tile_entity_json.at("id").get<TileEntityID>() == TileEntity::GHOST)
				++ghostCount;
		}
		renderer1.init(tilemap1);
		renderer2.init(tilemap2);
		renderer3.init(tilemap3);
		entities.clear();
		for (const auto &entity_json: json.at("entities"))
			(*entities.insert(Entity::fromJSON(entity_json)).first)->setRealm(shared);
		if (json.contains("extra"))
			extraData = json.at("extra");
	}

	void Realm::render(const int width, const int height, const Eigen::Vector2f &center, float scale, SpriteRenderer &sprite_renderer, float game_time) {
		renderer1.center = center;
		renderer1.scale  = scale;
		renderer1.onBackbufferResized(width, height);
		renderer2.center = center;
		renderer2.scale  = scale;
		renderer2.onBackbufferResized(width, height);
		renderer3.center = center;
		renderer3.scale  = scale;
		renderer3.onBackbufferResized(width, height);
		renderer1.render(outdoors? game_time : 1);
		renderer2.render(outdoors? game_time : 1);
		renderer3.render(outdoors? game_time : 1);
		for (const auto &entity: entities)
			if (!entity->isPlayer())
				entity->render(sprite_renderer);
		for (const auto &[index, tile_entity]: tileEntities)
			tile_entity->render(sprite_renderer);
		if (0 < ghostCount)
			sprite_renderer.drawOnScreen(cacheTexture("resources/checkmark.png"), width - 42.f, height - 42.f, 2.f);
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
		if (tile_entity->tileEntityID == TileEntity::GHOST)
			++ghostCount;
		tile_entity->onSpawn();
		return tile_entity;
	}

	void Realm::initEntities() {
		for (auto &entity: entities)
			entity->setRealm(shared_from_this());
	}

	void Realm::tick(float delta) {
		ticking = true;
		Game &game = getGame();
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

	TileEntityPtr Realm::tileEntityAt(const Position &position) const {
		const auto iter = tileEntities.find(position.row * tilemap1->width + position.column);
		if (iter == tileEntities.end())
			return {};
		return iter->second;
	}

	void Realm::remove(EntityPtr entity) {
		entities.erase(entity);
	}

	void Realm::remove(TileEntityPtr tile_entity) {
		const Position position = tile_entity->position;
		const Index index = getIndex(position);
		tileEntities.at(index)->onRemove();
		tileEntities.erase(index);
		setLayerHelper(index);
		if (tile_entity->getID() == TileEntity::GHOST)
			--ghostCount;
		updateNeighbors(position);
	}

	Position Realm::getPosition(Index index) const {
		return {index / getWidth(), index % getWidth()};
	}

	void Realm::onMoved(const EntityPtr &entity, const Position &position) {
		if (auto tile_entity = tileEntityAt(position))
			tile_entity->onOverlap(entity);
	}

	Game & Realm::getGame() {
		if (!game)
			throw std::runtime_error("Game is null for realm " + std::to_string(id));
		return *game;
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
		entity->init();
		entity->teleport(position);
	}

	void Realm::setLayer1(Index row, Index column, TileID tile) {
		(*tilemap1)(column, row) = tile;
		setLayerHelper(row, column);
	}

	void Realm::setLayer2(Index row, Index column, TileID tile) {
		(*tilemap2)(column, row) = tile;
		setLayerHelper(row, column);
	}

	void Realm::setLayer3(Index row, Index column, TileID tile) {
		(*tilemap3)(column, row) = tile;
		setLayerHelper(row, column);
	}

	void Realm::setLayer1(Index index, TileID tile) {
		tilemap1->tiles[index] = tile;
		setLayerHelper(index);
	}

	void Realm::setLayer2(Index index, TileID tile) {
		tilemap2->tiles[index] = tile;
		setLayerHelper(index);
	}

	void Realm::setLayer3(Index index, TileID tile) {
		tilemap3->tiles[index] = tile;
		setLayerHelper(index);
	}

	void Realm::setLayer1(const Position &position, TileID tile) {
		setLayer1(position.row, position.column, tile);
	}

	void Realm::setLayer2(const Position &position, TileID tile) {
		setLayer2(position.row, position.column, tile);
	}

	void Realm::setLayer3(const Position &position, TileID tile) {
		setLayer3(position.row, position.column, tile);
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
		return tilemap1->tiles[index];
	}

	TileID Realm::getLayer2(Index index) const {
		return tilemap2->tiles[index];
	}

	TileID Realm::getLayer3(Index index) const {
		return tilemap3->tiles[index];
	}

	TileID Realm::getLayer1(const Position &position) const {
		return (*tilemap1)(position);
	}

	TileID Realm::getLayer2(const Position &position) const {
		return (*tilemap2)(position);
	}

	TileID Realm::getLayer3(const Position &position) const {
		return (*tilemap3)(position);
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

		for (Index row_offset = -1; row_offset <= 1; ++row_offset)
			for (Index column_offset = -1; column_offset <= 1; ++column_offset)
				if (row_offset != 0 || column_offset != 0) {
					const Position offset_position = position + Position(row_offset, column_offset);
					if (!isValid(offset_position))
						continue;
					if (auto neighbor = tileEntityAt(offset_position)) {
						neighbor->onNeighborUpdated(-row_offset, -column_offset);
					} else {
						auto &tiles = tilemap2->tiles;
						TileID &tile = tiles.at(getIndex(offset_position));
						if (Monomap::woodenWalls.contains(tile)) {
							TileID march_result = march4([&](int8_t march_row_offset, int8_t march_column_offset) -> bool {
								const Position march_position = offset_position + Position(march_row_offset, march_column_offset);
								if (!isValid(march_position))
									return false;
								return Monomap::woodenWalls.contains(tiles.at(getIndex(march_position)));
							});

							const TileID marched = (march_result / 7 + 6) * (tilemap2->setWidth / tilemap2->tileSize) + march_result % 7;
							if (marched != tile) {
								tile = marched;
								layer2_updated = true;
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

		for (auto &[index, tile_entity]: tileEntities) {
			if (tile_entity->getID() != TileEntity::GHOST)
				continue;
			auto ghost = std::dynamic_pointer_cast<Ghost>(tile_entity);
			ghost->confirm();
			ghosts.push_back(ghost);
		}

		for (const auto &ghost: ghosts)
			remove(ghost);

		game->activateContext();
		renderer2.reupload();
	}

	void Realm::damageGround(const Position &position) {
		const Place place(position, shared_from_this(), nullptr);
		auto &game = getGame();

		if (auto iter = game.interactionSets.find(type); iter != game.interactionSets.end())
			iter->second->damageGround(place);
	}

	void Realm::toJSON(nlohmann::json &json) const {
		json["id"] = id;
		json["type"] = type;
		json["seed"] = seed;
		json["tilemap1"] = *tilemap1;
		json["tilemap2"] = *tilemap2;
		json["tilemap3"] = *tilemap3;
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

	bool Realm::isWalkable(Index row, Index column, const TileSet &tileset) const {
		if (!tileset.isWalkable((*tilemap1)(column, row)) || !tileset.isWalkable((*tilemap2)(column, row)) || !tileset.isWalkable((*tilemap3)(column, row)))
			return false;
		const Index index = getIndex(row, column);
		if (tileEntities.contains(index) && tileEntities.at(index)->solid)
			return false;
		return true;
	}

	void Realm::setLayerHelper(Index row, Index column) {
		const auto &tileset = tileSets.at(type);
		const Position position(row, column);
		pathMap[getIndex(position)] = isWalkable(row, column, *tileset);
		updateNeighbors(position);
	}

	void Realm::setLayerHelper(Index index) {
		const auto &tileset = tileSets.at(type);
		const Position position = getPosition(index);
		pathMap[index] = isWalkable(position.row, position.column, *tileset);
		updateNeighbors(position);
	}

	void Realm::resetPathMap() {
		const auto width = tilemap1->width;
		const auto height = tilemap1->height;
		pathMap.resize(width * height);
		const auto &tileset = tileSets.at(type);
		for (Index row = 0; row < height; ++row)
			for (Index column = 0; column < width; ++column)
				pathMap[getIndex(row, column)] = isWalkable(row, column, *tileset);
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

	void to_json(nlohmann::json &json, const Realm &realm) {
		realm.toJSON(json);
	}
}
