#include <iostream>

#include "Position.h"
#include "Tiles.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Sapling.h"
#include "realm/Realm.h"
#include "tileentity/Tree.h"

namespace Game3 {
	bool Sapling::use(Slot slot, ItemStack &stack, const std::shared_ptr<Player> &player, const Position &position) {
		auto &realm = *player->getRealm();
		const Index index = realm.getIndex(position);

		if (realm.type != Realm::OVERWORLD)
			return false;
	
		switch (realm.tilemap1->tiles[index]) {
			case Monomap::GRASS:
			case Monomap::GRASS_ALT1:
			case Monomap::GRASS_ALT2:
			case Monomap::LIGHT_GRASS:
			case Monomap::DIRT:
				break;
			default:
				return false;
		}

		if (realm.pathMap[index] && nullptr != realm.add(TileEntity::create<Tree>(Monomap::TREE1 + rand() % 3, Monomap::TREE0, position, 0.f))) {
			if (--stack.count == 0)
				player->inventory->erase(slot);
			player->inventory->notifyOwner();
			realm.pathMap[index] = 0;
			return true;
		}

		return false;
	}
}
