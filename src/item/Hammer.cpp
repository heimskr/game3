#include <iostream>

#include "Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Hammer.h"
#include "realm/Cave.h"
#include "tileentity/Building.h"

namespace Game3 {
	bool Hammer::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		auto &realm = *place.realm;

		if (auto building = std::dynamic_pointer_cast<Building>(realm.tileEntityAt(place.position)); building && building->tileID == "base:tile/cave"_id) {
			Game &game = realm.getGame();
			const RealmID realm_id = building->innerRealmID;

			if (auto cave_realm = std::dynamic_pointer_cast<Cave>(game.getRealm(realm_id))) {
				if (--cave_realm->entranceCount == 0)
					game.removeRealm(realm_id);
				realm.remove(building);
				if (stack.reduceDurability())
					place.player->getInventory()->erase(slot);
				return true;
			}

			throw std::runtime_error("Cave entrance leads to realm " + std::to_string(realm_id) + ", which isn't a cave");
		}

		Player &player = *place.player;
		Inventory &inventory = *player.getInventory();
		Tileset &tileset = place.realm->getTileset();

		for (auto iter = mainLayers.rbegin(); iter != mainLayers.rend(); ++iter) {
			const auto tile = place.getName(*iter);
			ItemStack equivalent;
			if (tile && tileset.getItemStack(place.getGame(), *tile, equivalent)) {
				place.set(*iter, tileset.getEmpty());
				if (stack.reduceDurability())
					inventory.erase(slot);
				player.give(equivalent);
				inventory.notifyOwner();
				place.realm->queueReupload(*iter);
				return true;
			}
		}

		return false;
	}
}
