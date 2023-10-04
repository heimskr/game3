#include <iostream>

#include "Log.h"
#include "graphics/Tileset.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "realm/Cave.h"
#include "tileentity/Building.h"
#include "worldgen/CaveGen.h"

namespace Game3 {
	Cave::Cave(Game &game_, RealmID id_, RealmID parent_realm, int seed_):
		Realm(game_, id_, ID(), "base:tileset/monomap", seed_), parentRealm(parent_realm) {}

	void Cave::onRemove() {
		// Assumptions:
		// - All entrances to a given cave realm appear in exactly one realm.
		//    -> If we find a cave entrance in one realm, we don't need to search other realms for another entrance to the same cave.
		// - All cave entrances in a given realm lead to the same cave.
		//    -> If we find one cave entrance in a realm, we can stop after destroying its linked cave and we don't have to look for more entrances.
		Game &game = getGame();
		for (const auto &[index, tile_entity]: tileEntities) {
			if (tile_entity->tileID != "base:tile/cave")
				continue;
			if (auto building = std::dynamic_pointer_cast<Building>(tile_entity)) {
				if (auto cave_realm = std::dynamic_pointer_cast<Cave>(game.getRealm(building->innerRealmID)))
					game.removeRealm(building->innerRealmID);
				else
					WARN("Cave entrance leads to realm " << building->innerRealmID << ", which isn't a cave. Not erasing.");
				break;
			}
		}
	}

	bool Cave::interactGround(const std::shared_ptr<Player> &player, const Position &position, Modifiers modifiers) {
		if (Realm::interactGround(player, position, modifiers))
			return true;

		std::optional<ItemStack> ore_stack;

		const TileID tile2 = getTile(Layer::Objects, position);
		const auto &tileset = getTileset();
		const Identifier &tile_id = tileset[tile2];

		Game &game = getGame();

		bool grim = false;

		static std::unordered_map<Identifier, std::pair<Identifier, bool>> ores{
			{"base:tile/cave_coal",     {"base:item/coal",        false}},
			{"base:tile/cave_copper",   {"base:item/copper_ore",  false}},
			{"base:tile/cave_diamond",  {"base:item/diamond_ore", false}},
			{"base:tile/cave_gold",     {"base:item/gold_ore",    false}},
			{"base:tile/cave_iron",     {"base:item/iron_ore",    false}},
			{"base:tile/cave_wall",     {"base:item/stone",       false}},
			{"base:tile/grimstone",     {"base:item/grimstone",    true}},
			{"base:tile/grim_diamond",  {"base:item/diamond_ore",  true}},
			{"base:tile/grim_uranium",  {"base:item/uranium_ore",  true}},
			{"base:tile/grim_fireopal", {"base:item/fire_opal",    true}},
		};

		if (auto iter = ores.find(tile_id); iter != ores.end()) {
			ore_stack.emplace(game, iter->second.first, 1);
			grim = iter->second.second;
		}

		if (ore_stack) {
			const InventoryPtr inventory = player->getInventory();
			if (auto *stack = inventory->getActive()) {
				if (stack->hasAttribute("base:attribute/pickaxe") && !inventory->add(*ore_stack)) {
					reveal(position);
					setTile(Layer::Objects, position, 0);
					if (getTile(Layer::Terrain, position) == 0)
						setTile(Layer::Terrain, position, grim? "base:tile/grimdirt" : "base:tile/cave_dirt", true);
					if (stack->reduceDurability())
						inventory->erase(inventory->activeSlot);
					return true;
				}
			}
		}

		return false;
	}

	void Cave::reveal(const Position &position) {
		const auto &tileset = getTileset();
		const TileID empty_id = tileset.getEmptyID();
		if (getTile(Layer::Objects, position) != empty_id) {
			const TileID void_id = tileset["base:tile/void"];
			for (Index row_offset = -1; row_offset <= 1; ++row_offset)
				for (Index column_offset = -1; column_offset <= 1; ++column_offset)
					if (row_offset != 0 || column_offset != 0)
						if (const Position offset_position = position + Position(row_offset, column_offset); getTile(Layer::Highest, offset_position) == void_id)
							setTile(Layer::Highest, offset_position, empty_id);
		}
	}

	void Cave::generateChunk(const ChunkPosition &chunk_position) {
		auto rng = chunk_position.getRNG();
		tileProvider.ensureAllChunks(chunk_position);
		WorldGen::generateCave(shared_from_this(), rng, seed, {chunk_position, chunk_position});
		tileProvider.updateChunk(chunk_position);
	}

	void Cave::absorbJSON(const nlohmann::json &json, bool full_data) {
		Realm::absorbJSON(json, full_data);
		parentRealm = json.at("parentRealm");
		if (auto iter = json.find("entranceCount"); iter != json.end())
			entranceCount = iter->get<size_t>();
		else
			entranceCount = 1;
	}

	void Cave::toJSON(nlohmann::json &json, bool full_data) const {
		Realm::toJSON(json, full_data);
		json["parentRealm"] = parentRealm;
		if (entranceCount != 1)
			json["entranceCount"] = entranceCount.load();
	}
}
