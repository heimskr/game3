#include <iostream>

#include "Position.h"
#include "Tiles.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Furniture.h"
#include "realm/Realm.h"
#include "tileentity/Ghost.h"

namespace Game3 {
	bool Furniture::use(Slot slot, ItemStack &stack, const std::shared_ptr<Player> &player, const Position &position) {
		auto &realm = *player->getRealm();

		if (realm.pathMap[realm.getIndex(position)] && !realm.hasTileEntityAt(position) && nullptr != realm.add(TileEntity::create<Ghost>(position, stack.withCount(1)))) {
			if (--stack.count == 0)
				player->inventory->erase(slot);
			player->inventory->notifyOwner();
			return true;
		}

		return false;
	}
}
