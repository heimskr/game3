#include "Log.h"
#include "Position.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "tile/FarmlandTile.h"

namespace Game3 {
	FarmlandTile::FarmlandTile():
		Tile(ID()) {}

	bool FarmlandTile::interact(const Place &place, Layer layer) {
		PlayerPtr player = place.player;
		if (!player)
			return false;

		InventoryPtr inventory = player->getInventory();
		if (!inventory)
			return false;

		{
			auto inventory_lock = inventory->uniqueLock();

			if (ItemStack *active = inventory->getActive(); active && active->hasAttribute("base:attribute/pickaxe")) {
				if (active->reduceDurability())
					inventory->erase(inventory->activeSlot);
				inventory->notifyOwner();
				place.set(layer, "base:tile/dirt");
				return true;
			}
		}

		return Tile::interact(place, layer);
	}
}
