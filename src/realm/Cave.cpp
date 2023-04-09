#include <iostream>

#include "Tileset.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "realm/Cave.h"
#include "tileentity/Building.h"

namespace Game3 {
	Cave::Cave(RealmID id_, RealmID parent_realm, TilemapPtr tilemap1_, TilemapPtr tilemap2_, TilemapPtr tilemap3_, BiomeMapPtr biome_map, int seed_):
		Realm(id_, Realm::CAVE, std::move(tilemap1_), std::move(tilemap2_), std::move(tilemap3_), std::move(biome_map), seed_), parentRealm(parent_realm) {}

	Cave::Cave(RealmID id_, RealmID parent_realm, TilemapPtr tilemap1_, BiomeMapPtr biome_map, int seed_):
		Realm(id_, Realm::CAVE, std::move(tilemap1_), std::move(biome_map), seed_), parentRealm(parent_realm) {}

	Cave::~Cave() {
		// Assumptions:
		// - All entrances to a given cave realm appear in exactly one realm.
		//    -> If we find a cave entrance in one realm, we don't need to search other realms for another entrance to the same cave.
		// - All cave entrances in a given realm lead to the same cave.
		//    -> If we find one cave entrance in a realm, we can stop after destroying its linked cave and we don't have to look for more entrances.
		auto &game = getGame();
		for (const auto &[index, tile_entity]: tileEntities) {
			if (tile_entity->tileID != Monomap::CAVE)
				continue;
			if (auto building = std::dynamic_pointer_cast<Building>(tile_entity)) {
				if (auto cave_realm = std::dynamic_pointer_cast<Cave>(game.realms.at(building->innerRealmID)))
					game.realms.erase(building->innerRealmID);
				else
					std::cerr << "Cave entrance leads to realm " + std::to_string(building->innerRealmID) + ", which isn't a cave. Not erasing.\n";
				break;
			}
		}
	}

	bool Cave::interactGround(const std::shared_ptr<Player> &player, const Position &position) {
		if (Realm::interactGround(player, position))
			return true;

		const Index index = getIndex(position);

		std::optional<ItemStack> ore_stack;

		const TileID tile2 = tilemap2->tiles.at(index);
		if (tile2 == Monomap::CAVE_COAL)
			ore_stack.emplace(Item::COAL, 1);
		else if (tile2 == Monomap::CAVE_COPPER)
			ore_stack.emplace(Item::COPPER_ORE, 1);
		else if (tile2 == Monomap::CAVE_DIAMOND)
			ore_stack.emplace(Item::DIAMOND_ORE, 1);
		else if (tile2 == Monomap::CAVE_GOLD)
			ore_stack.emplace(Item::GOLD_ORE, 1);
		else if (tile2 == Monomap::CAVE_IRON)
			ore_stack.emplace(Item::IRON_ORE, 1);
		else if (tile2 == Monomap::CAVE_WALL)
			ore_stack.emplace(Item::STONE, 1);

		if (ore_stack) {
			Inventory &inventory = *player->inventory;
			if (auto *stack = inventory.getActive()) {
				if (stack->has(ItemAttribute::Pickaxe) && !inventory.add(*ore_stack)) {
					setLayer2(index, Monomap::EMPTY);
					getGame().activateContext();
					renderer2.reupload();
					reveal(position);
					if (stack->reduceDurability())
						inventory.erase(inventory.activeSlot);
					return true;
				}
			}
		}

		return false;
	}

	void Cave::reveal(const Position &position) {
		if (!isValid(position))
			return;

		if (tilemap2->tiles.at(getIndex(position)) == Monomap::EMPTY) {
			bool changed = false;
			for (Index row_offset = -1; row_offset <= 1; ++row_offset)
				for (Index column_offset = -1; column_offset <= 1; ++column_offset)
					if (row_offset != 0 || column_offset != 0) {
						const Position offset_position = position + Position(row_offset, column_offset);
						if (!isValid(offset_position))
							continue;
						TileID &tile3 = tilemap3->tiles.at(getIndex(offset_position));
						if (tile3 == Monomap::VOID) {
							tile3 = Monomap::EMPTY;
							changed = true;
						}
					}
			if (changed) {
				getGame().activateContext();
				renderer3.reupload();
			}
		}
	}

	void Cave::absorbJSON(const nlohmann::json &json) {
		Realm::absorbJSON(json);
		parentRealm = json.at("parentRealm");
		entranceCount = json.contains("entranceCount")? json.at("entranceCount").get<decltype(entranceCount)>() : 1;
	}

	void Cave::toJSON(nlohmann::json &json) const {
		Realm::toJSON(json);
		json["parentRealm"] = parentRealm;
		if (entranceCount != 1)
			json["entranceCount"] = entranceCount;
	}
}
