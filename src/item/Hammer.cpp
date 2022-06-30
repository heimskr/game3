#include <iostream>

#include "Position.h"
#include "Tiles.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Hammer.h"
#include "realm/Cave.h"
#include "tileentity/Building.h"

namespace Game3 {
	bool Hammer::use(Slot slot, ItemStack &stack, const std::shared_ptr<Player> &player, const Position &position) {
		Realm &realm = *player->getRealm();

		if (auto building = std::dynamic_pointer_cast<Building>(realm.tileEntityAt(position))) {
			if (building->tileID == Monomap::CAVE) {
				Game &game = realm.getGame();
				const RealmID realm_id = building->innerRealmID;
				if (auto cave_realm = std::dynamic_pointer_cast<Cave>(game.realms.at(realm_id))) {
					if (--cave_realm->entranceCount == 0)
						game.realms.erase(realm_id);
					realm.remove(building);
					if (stack.reduceDurability())
						player->inventory->erase(slot);
				} else
					throw std::runtime_error("Cave entrance leads to realm " + std::to_string(realm_id) + ", which isn't a cave");
			}
		}

		return false;
	}
}
