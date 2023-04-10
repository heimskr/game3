#include <iostream>
#include "MarchingSquares.h"
#include "Tileset.h"
#include "entity/EntityFactory.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "error/OverlapError.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "recipe/CraftingRecipe.h"
#include "tileentity/CraftingStation.h"
#include "tileentity/Ghost.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "ui/SpriteRenderer.h"
#include "ui/tab/InventoryTab.h"

#include "registry/Registries.h"

namespace Game3 {
	GhostFunction::GhostFunction(Identifier identifier_, decltype(function) function_):
		NamedRegisterable(std::move(identifier_)),
		function(function_) {}

	bool GhostFunction::operator()(const Identifier &tilename, const Place &place) const {
		return function(tilename, place);
	}

	static void spawnCauldron(const Place &place) {
		// place.realm->add(TileEntity::create<CraftingStation>(Monomap::CAULDRON_RED_FULL, place.position, Identifier("base", "cauldron_station")));
		place.realm->setLayer2(place.position, "base:tile/cauldron_red_full");
	}

	static void spawnPurifier(const Place &place) {
		// place.realm->add(TileEntity::create<CraftingStation>(Monomap::PURIFIER, place.position, Identifier("base", "purifier_station")));
		place.realm->setLayer2(place.position, "base:tile/purifier");
	}

	void initGhosts(Game &game) {
		game.add(std::make_shared<GhostDetails>("base:cauldron", spawnCauldron, "base:tile/cauldron_red_full"));
		game.add(std::make_shared<GhostDetails>("base:purifier", spawnPurifier, "base:tile/purifier"));
	}

	GhostDetails & GhostDetails::get(const Game &game, const ItemStack &stack) {
		const auto &registry = game.registry<GhostDetailsRegistry>();
		if (auto iter = registry.items.find(stack.item->identifier); iter != registry.items.end())
			return *iter->second;
		throw std::runtime_error("Couldn't get GhostDetails for " + stack.item->name);
	}

	void from_json(const nlohmann::json &json, GhostDetails &details) {
		details.type = json.at(0);
		details.useMarchingSquares = json.at(1);
		details.columnsPerRow = json.at(2);
		details.rowOffset = json.at(3);
		details.columnOffset = json.at(4);
	}

	Ghost::Ghost(const Place &place, ItemStack material_):
		TileEntity("base:tile/missing", ID(), place.position, true),
		details(GhostDetails::get(place.getGame(), material_)),
		material(std::move(material_)) {}

	void Ghost::toJSON(nlohmann::json &json) const {
		TileEntity::toJSON(json);
		json["material"] = material;
	}

	void Ghost::absorbJSON(Game &game, const nlohmann::json &json) {
		TileEntity::absorbJSON(game, json);
		material = ItemStack::fromJSON(game, json.at("material"));
		details  = GhostDetails::get(game, material);
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
		const auto &tileset = *tilemap.tileset;
		const auto tilesize = tilemap.tileSize;
		const auto column_count = tilemap.setWidth / tilesize;

		TileID tile_id = tileset.getEmptyID();

		if (details.customFn)
			tile_id = tileset[details.customTileName];
		else if (details.useMarchingSquares)
			tile_id = marched;
		else
			tile_id = details.rowOffset * column_count + details.columnOffset;

		const auto x = (tile_id % column_count) * tilesize;
		const auto y = (tile_id / column_count) * tilesize;
		sprite_renderer.drawOnMap(*tilemap.texture, position.column, position.row, x / 2, y / 2, tilesize, tilesize, 1.f, 0.f, .5f);
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
				if (auto *ghost = dynamic_cast<Ghost *>(tile_entity.get()); ghost && *ghost->material.item == *material.item)
					return true;
			return std::nullopt;
		};

		const auto &registry = realm->getGame().registry<GhostFunctionRegistry>();
		const auto &fn = *registry[details.type];
		const auto &tileset = *realm->tilemap2->tileset;

		march_result = march4([&](int8_t row_offset, int8_t column_offset) -> bool {
			const Position offset_position(position + Position(row_offset, column_offset));
			if (auto value = check(offset_position))
				return *value;
			return fn(tileset[tiles.at(realm->getIndex(offset_position))], Place(offset_position, realm, nullptr));
		});

		const TileID marched_row = march_result / 7;
		const TileID marched_column = march_result % 7;
		const TileID row = marched_row + details.rowOffset;
		const TileID column = marched_column + details.columnOffset;
		marched = row * details.columnsPerRow + column;
	}

	void Ghost::confirm() {
		auto realm = getRealm();
		auto &tilemap2 = *realm->tilemap2;
		const auto &tileset = *tilemap2.tileset;

		if (tilemap2(position) != tileset.getEmptyID())
			throw OverlapError("Can't confirm ghost at " + std::string(position));

		if (details.customFn) {
			details.customFn({position, realm, nullptr});
		} else {
			TileID tile_id = tileset[tileset.getMissing()];
			if (details.useMarchingSquares)
				tile_id = marched;
			else
				tile_id = details.rowOffset * (tilemap2.setWidth / tilemap2.tileSize) + details.columnOffset;

			realm->setLayer2(position, tile_id);
		}
	}
}
