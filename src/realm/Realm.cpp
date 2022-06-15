#include <iostream>
#include <thread>
#include <unordered_set>

#include "Tiles.h"
#include "entity/Entity.h"
#include "game/Game.h"
#include "realm/Keep.h"
#include "realm/Realm.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/Sign.h"
#include "tileentity/Teleporter.h"
#include "util/Timer.h"
#include "util/Util.h"
#include "worldgen/Carpet.h"
#include "worldgen/House.h"
#include "worldgen/Keep.h"

// Rather ugly code here.

namespace Game3 {
	std::unordered_map<RealmType, Texture> Realm::textureMap {
		{Realm::OVERWORLD, Texture("resources/tileset2.png")},
		{Realm::HOUSE,     Texture("resources/house.png")},
		{Realm::KEEP,      Texture("resources/house.png")},
	};

	Realm::Realm(RealmID id_, RealmType type_, const std::shared_ptr<Tilemap> &tilemap1_, const std::shared_ptr<Tilemap> &tilemap2_, const std::shared_ptr<Tilemap> &tilemap3_):
	id(id_), type(type_), tilemap1(tilemap1_), tilemap2(tilemap2_), tilemap3(tilemap3_) {
		tilemap1->init();
		tilemap2->init();
		tilemap3->init();
		renderer1.init(tilemap1);
		renderer2.init(tilemap2);
		renderer3.init(tilemap3);
		resetPathMap();
	}

	Realm::Realm(RealmID id_, RealmType type_, const std::shared_ptr<Tilemap> &tilemap1_): id(id_), type(type_), tilemap1(tilemap1_) {
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

	std::shared_ptr<Realm> Realm::fromJSON(const nlohmann::json &json) {
		std::shared_ptr<Realm> out;
		const RealmType type = json.at("type");
		switch (type) {
			case Realm::OVERWORLD:
			case Realm::HOUSE:
			case Realm::BLACKSMITH:
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
		tilemap1 = std::make_shared<Tilemap>(json.at("tilemap1"));
		tilemap2 = std::make_shared<Tilemap>(json.at("tilemap2"));
		tilemap3 = std::make_shared<Tilemap>(json.at("tilemap3"));
		tilemap1->texture.init();
		tilemap2->texture.init();
		tilemap3->texture.init();
		outdoors = json.at("outdoors");
		for (const auto &[index, tile_entity_json]: json.at("tileEntities").get<std::unordered_map<std::string, nlohmann::json>>())
			tileEntities.emplace(parseUlong(index), TileEntity::fromJSON(tile_entity_json)).first->second->setRealm(shared);
		renderer1.init(tilemap1);
		renderer2.init(tilemap2);
		renderer3.init(tilemap3);
		entities.clear();
		for (const auto &entity_json: json.at("entities"))
			(*entities.insert(Entity::fromJSON(entity_json)).first)->setRealm(shared).initAfterRealm();
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

	std::shared_ptr<Entity> Realm::add(const std::shared_ptr<Entity> &entity) {
		entity->setRealm(shared_from_this());
		entities.insert(entity);
		return entity;
	}

	std::shared_ptr<TileEntity> Realm::add(const std::shared_ptr<TileEntity> &tile_entity) {
		const Index index = getIndex(tile_entity->position);
		if (tileEntities.contains(index))
			return nullptr;
		tile_entity->setRealm(shared_from_this());
		tileEntities.emplace(index, tile_entity);
		if (tile_entity->solid)
			pathMap[index] = false;
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
		ticking = false;
		for (const auto &entity: removalQueue)
			remove(entity);
		removalQueue.clear();
	}

	std::vector<std::shared_ptr<Entity>> Realm::findEntities(const Position &position) const {
		std::vector<std::shared_ptr<Entity>> out;
		for (const auto &entity: entities)
			if (entity->position == position)
				out.push_back(entity);
		return out;
	}

	std::vector<std::shared_ptr<Entity>> Realm::findEntities(const Position &position, const std::shared_ptr<Entity> &except) const {
		std::vector<std::shared_ptr<Entity>> out;
		for (const auto &entity: entities)
			if (entity->position == position && entity != except)
				out.push_back(entity);
		return out;
	}

	std::shared_ptr<Entity> Realm::findEntity(const Position &position) const {
		for (const auto &entity: entities)
			if (entity->position == position)
				return entity;
		return {};
	}

	std::shared_ptr<Entity> Realm::findEntity(const Position &position, const std::shared_ptr<Entity> &except) const {
		for (const auto &entity: entities)
			if (entity->position == position && entity != except)
				return entity;
		return {};
	}

	std::shared_ptr<TileEntity> Realm::tileEntityAt(const Position &position) const {
		const auto iter = tileEntities.find(position.row * tilemap1->width + position.column);
		if (iter == tileEntities.end())
			return {};
		return iter->second;
	}

	void Realm::remove(const std::shared_ptr<Entity> &entity) {
		entities.erase(entity);
	}

	void Realm::remove(const std::shared_ptr<TileEntity> &tile_entity) {
		tileEntities.erase(getIndex(tile_entity->position));
	}

	Position Realm::getPosition(Index index) const {
		return {index / getWidth(), index % getWidth()};
	}

	void Realm::onMoved(const std::shared_ptr<Entity> &entity, const Position &position) {
		if (auto tile_entity = tileEntityAt(position))
			tile_entity->onOverlap(entity);
	}

	Game & Realm::getGame() {
		if (!game)
			throw std::runtime_error("Game is null for realm " + std::to_string(id));
		return *game;
	}

	void Realm::queueRemoval(const std::shared_ptr<Entity> &entity) {
		if (ticking)
			removalQueue.push_back(entity);
		else
			remove(entity);
	}

	void Realm::absorb(const std::shared_ptr<Entity> &entity, const Position &position) {
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

	bool Realm::interactGround(const std::shared_ptr<Player> &player, const Position &position) {
		switch (type) {
			case Realm::OVERWORLD: {
				const TileID tile = tilemap1->tiles.at(getIndex(position));
				std::optional<ItemID> item;
				std::optional<ItemAttribute> attribute;

				if (tile == OverworldTiles::SAND) {
					item.emplace(Item::SAND);
					attribute.emplace(ItemAttribute::Shovel);
				} else if (tile == OverworldTiles::STONE) {
					item.emplace(Item::STONE);
					attribute.emplace(ItemAttribute::Pickaxe);
				}

				if (item && attribute) {
					auto &inventory = *player->inventory;
					if (auto *stack = inventory.getActive()) {
						if (stack->has(*attribute) && !inventory.add({*item, 1})) {
							if (stack->reduceDurability())
								inventory.erase(inventory.activeSlot);
							return true;
						}
					}
				}

				break;
			}
		}

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

	void Realm::toJSON(nlohmann::json &json) const {
		json["id"] = id;
		json["type"] = type;
		json["tilemap1"] = *tilemap1;
		json["tilemap2"] = *tilemap2;
		json["tilemap3"] = *tilemap3;
		json["outdoors"] = outdoors;
		json["tileEntities"] = std::unordered_map<std::string, nlohmann::json>();
		for (const auto &[index, tile_entity]: tileEntities)
			json["tileEntities"][std::to_string(index)] = *tile_entity;
		json["entities"] = std::vector<nlohmann::json>();
		for (const auto &entity: entities)
			json["entities"].push_back(entity->toJSON());
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
		pathMap[getIndex(row, column)] = isWalkable(row, column, *tileset);
	}

	void Realm::setLayerHelper(Index index) {
		const auto &tileset = tileSets.at(type);
		pathMap[index] = isWalkable(index / getWidth(), index % getWidth(), *tileset);
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

	void to_json(nlohmann::json &json, const Realm &realm) {
		realm.toJSON(json);
	}
}
