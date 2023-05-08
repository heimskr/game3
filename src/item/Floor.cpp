#include "Position.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Floor.h"
#include "realm/Realm.h"

namespace Game3 {
	Floor::Floor(ItemID identifier_, std::string name_, Identifier tilename_, MoneyCount base_price, ItemCount max_count):
		Item(std::move(identifier_), std::move(name_), base_price, max_count),
		tilename(std::move(tilename_)) {}

	bool Floor::use(Slot slot, ItemStack &stack, const Place &place, Modifiers) {
		auto &realm = *place.realm;
		const auto &tileset = realm.getTileset();

		if (place.getName(1) == tileset.getEmpty()) {
			place.set(1, tilename);
			realm.reupload(1);
			if (--stack.count == 0)
				place.player->inventory->erase(slot);
			place.player->inventory->notifyOwner();
			return true;
		}

		return false;
	}
}
