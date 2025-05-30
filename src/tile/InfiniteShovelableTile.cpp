#include "entity/Player.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "tile/InfiniteShovelableTile.h"

namespace Game3 {
	InfiniteShovelableTile::InfiniteShovelableTile(Identifier identifier, Identifier itemID):
		Tile(std::move(identifier)),
		itemID(std::move(itemID)) {}

	bool InfiniteShovelableTile::interact(const Place &place, Layer layer, const ItemStackPtr &used_item, Hand hand) {
		PlayerPtr player = place.player;
		InventoryPtr inventory = player->getInventory(0);
		std::unique_lock inventory_lock = inventory->uniqueLock();

		ItemStackPtr active = used_item? used_item : inventory->getActive();

		if (active && active->hasAttribute("base:attribute/shovel")) {
			if (player->hasTooldown()) {
				return true;
			}

			player->setTooldown(1, active);
			player->give(ItemStack::create(place.getGame(), itemID, 1));

			if (active->reduceDurability()) {
				inventory->erase(used_item? player->getHeldSlot(hand) : inventory->activeSlot.load());
			}

			return true;
		}

		return false;
	}
}
