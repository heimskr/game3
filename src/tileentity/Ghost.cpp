#include <iostream>
#include "MarchingSquares.h"
#include "Tileset.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "error/OverlapError.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "tileentity/CraftingStation.h"
#include "tileentity/Ghost.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/InventoryTab.h"

namespace Game3 {
	static void spawnCauldron(const Place &place) {
		place.realm->add(TileEntity::create<CraftingStation>(Monomap::CAULDRON_RED_FULL, place.position, Identifier("base", "cauldron_station")));
		place.realm->setLayer2(place.position, Monomap::CAULDRON_RED_FULL);
	}

	static void spawnPurifier(const Place &place) {
		place.realm->add(TileEntity::create<CraftingStation>(Monomap::PURIFIER, place.position, Identifier("base", "purifier_station")));
		place.realm->setLayer2(place.position, Monomap::PURIFIER);
	}

	GhostDetails GhostDetails::WOODEN_WALL {GhostType::WoodenWall, true,  32,  6, 0};
	GhostDetails GhostDetails::PLANT_POT1  {GhostType::Normal,     false, 32, 12, 4};
	GhostDetails GhostDetails::PLANT_POT2  {GhostType::Normal,     false, 32, 12, 5};
	GhostDetails GhostDetails::PLANT_POT3  {GhostType::Normal,     false, 32, 12, 6};
	GhostDetails GhostDetails::TOWER       {GhostType::Tower,      true,  32,  6, 7};
	GhostDetails GhostDetails::CAULDRON    {spawnCauldron, Monomap::CAULDRON_RED_FULL};
	GhostDetails GhostDetails::PURIFIER    {spawnPurifier, Monomap::PURIFIER};

	GhostDetails & GhostDetails::get(const ItemStack &stack) {
		switch (stack.item->id) {
			case Item::WOODEN_WALL: return WOODEN_WALL;
			case Item::PLANT_POT1:  return PLANT_POT1;
			case Item::PLANT_POT2:  return PLANT_POT2;
			case Item::PLANT_POT3:  return PLANT_POT3;
			case Item::TOWER:       return TOWER;
			case Item::CAULDRON:    return CAULDRON;
			case Item::PURIFIER:    return PURIFIER;
			default: throw std::runtime_error("Couldn't get GhostDetails for " + stack.item->name);
		}
	}

	void Ghost::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["material"] = material;
	}

	void Ghost::absorbJSON(const nlohmann::json &json) {
		TileEntity::absorbJSON(json);
		material = json.at("material");
		details = GhostDetails::get(material);
	}

	void Ghost::onSpawn() {
		march();
		updateNeighbors();
	}

	void Ghost::onNeighborUpdated(Index row_offset, Index column_offset) {
		if (row_offset == 0 || column_offset == 0)
			march();
	}

	bool Ghost::onInteractNextTo(const std::shared_ptr<Player> &player) {
		auto &inventory = *player->inventory;
		auto realm = getRealm();
		if (auto leftover = inventory.add(material))
			leftover->spawn(realm, position);
		realm->remove(shared_from_this());
		return true;
	}

	void Ghost::render(SpriteRenderer &sprite_renderer) {
		if (!isVisible())
			return;

		auto realm = getRealm();
		auto &tilemap = *realm->tilemap2;
		const auto tilesize = tilemap.tileSize;
		const auto column_count = tilemap.setWidth / tilesize;

		TileID tile_id = Monomap::MISSING;

		if (details.customFn)
			tile_id = details.customTileID;
		else if (details.useMarchingSquares)
			tile_id = marched;
		else
			tile_id = details.rowOffset * column_count + details.columnOffset;

		const auto x = (tile_id % column_count) * tilesize;
		const auto y = (tile_id / column_count) * tilesize;
		sprite_renderer.drawOnMap(tilemap.texture, position.column, position.row, x / 2, y / 2, tilesize, tilesize, 1.f, 0.f, .5f);
	}

	void Ghost::march() {
		if (!details.useMarchingSquares)
			return;

		auto realm = getRealm();
		TileID march_result;
		const auto &tiles = realm->tilemap2->tiles;

		auto check = [&](const Position &offset_position) -> std::optional<bool> {
			if (!realm->isValid(offset_position))
				return false;
			if (auto tile_entity = realm->tileEntityAt(offset_position))
				if (tile_entity->getID() == TileEntity::GHOST)
					if (*dynamic_cast<Ghost *>(tile_entity.get())->material.item == *material.item)
						return true;
			return std::nullopt;
		};

		switch (details.type) {
			case GhostType::WoodenWall:
				march_result = march4([&](int8_t row_offset, int8_t column_offset) -> bool {
					const Position offset_position(position + Position(row_offset, column_offset));
					if (auto value = check(offset_position))
						return *value;
					return monomap.woodenWalls.contains(tiles.at(realm->getIndex(offset_position)));
				});
				break;
			case GhostType::Tower:
				march_result = march4([&](int8_t row_offset, int8_t column_offset) -> bool {
					const Position offset_position(position + Position(row_offset, column_offset));
					if (auto value = check(offset_position))
						return *value;
					return monomap.towerSet.contains(tiles.at(realm->getIndex(offset_position)));
				});
				break;
			default:
				throw std::runtime_error("Unhandled GhostType in Ghost::march(): " + std::to_string(static_cast<int>(details.type)));
		};

		const TileID marched_row = march_result / 7;
		const TileID marched_column = march_result % 7;
		const TileID row = marched_row + details.rowOffset;
		const TileID column = marched_column + details.columnOffset;
		marched = row * details.columnsPerRow + column;
	}

	void Ghost::confirm() {
		auto realm = getRealm();
		auto &tilemap2 = *realm->tilemap2;
		if (tilemap2(position) != tileSets.at(realm->type)->getEmpty())
			throw OverlapError("Can't confirm ghost at " + std::string(position));

		if (details.customFn) {
			details.customFn({position, realm, nullptr});
		} else {
			TileID tile_id = Monomap::MISSING;
			if (details.useMarchingSquares)
				tile_id = marched;
			else
				tile_id = details.rowOffset * (tilemap2.setWidth / tilemap2.tileSize) + details.columnOffset;

			realm->setLayer2(position, tile_id);
		}
	}
}
