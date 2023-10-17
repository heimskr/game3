#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Floor.h"
#include "realm/Realm.h"

namespace Game3 {
	Floor::Floor(ItemID identifier_, std::string name_, Identifier tilename_, MoneyCount base_price, ItemCount max_count):
		Item(std::move(identifier_), std::move(name_), base_price, max_count),
		tilename(std::move(tilename_)) {}

	bool Floor::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		auto &realm = *place.realm;
		const auto &tileset = realm.getTileset();

		if (place.getName(Layer::Terrain) == tileset.getEmpty()) {
			place.set(Layer::Terrain, tilename);
			const InventoryPtr inventory = place.player->getInventory();
			auto inventory_lock = inventory->uniqueLock();
			if (--stack.count == 0)
				inventory->erase(slot);
			inventory->notifyOwner();
			return true;
		}

		return false;
	}

	bool Floor::drag(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers) {
		return use(slot, stack, place, modifiers, {0.f, 0.f});
	}
}
