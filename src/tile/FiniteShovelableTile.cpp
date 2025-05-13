#include "entity/Player.h"
#include "game/Inventory.h"
#include "realm/Realm.h"
#include "tile/FiniteShovelableTile.h"

namespace Game3 {
	FiniteShovelableTile::FiniteShovelableTile(Identifier identifier, Identifier itemID):
		Tile(std::move(identifier)),
		itemID(std::move(itemID)) {}

	bool FiniteShovelableTile::interact(const Place &place, Layer layer, const ItemStackPtr &used_item, Hand hand) {
		if (layer != Layer::Submerged) {
			return false;
		}

		PlayerPtr player = place.player;
		InventoryPtr inventory = player->getInventory(0);
		std::unique_lock inventory_lock = inventory->uniqueLock();

		ItemStackPtr active = used_item? used_item : inventory->getActive();

		if (active && active->hasAttribute("base:attribute/shovel")) {
			place.realm->setTile(layer, place.position, 0);
			player->give(ItemStack::create(place.getGame(), itemID, 1));

			if (active->reduceDurability()) {
				inventory->erase(used_item? player->getHeldSlot(hand) : inventory->activeSlot.load());
			}

			return true;
		}

		return false;
	}
}
