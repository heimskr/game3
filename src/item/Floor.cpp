#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Floor.h"
#include "realm/Realm.h"

namespace Game3 {
	Floor::Floor(ItemID identifier, std::string name, Identifier tilename, MoneyCount basePrice, ItemCount maxCount):
		Item(std::move(identifier), std::move(name), basePrice, maxCount),
		tilename(std::move(tilename)) {}

	bool Floor::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		Realm &realm = *place.realm;
		const Tileset &tileset = realm.getTileset();

		if (place.get(Layer::Flooring) == tileset.getEmptyID()) {
			place.set(Layer::Flooring, tilename);
			InventoryPtr inventory = place.player->getInventory(0);
			assert(inventory != nullptr);
			inventory->decrease(stack, slot, 1, true);
			return true;
		}

		return false;
	}

	bool Floor::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, DragAction) {
		return use(slot, stack, place, modifiers, offsets);
	}
}
