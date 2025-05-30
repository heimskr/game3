#include "util/Log.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "graphics/Tileset.h"
#include "item/Seed.h"
#include "realm/Realm.h"
#include "types/Position.h"

namespace Game3 {
	Seed::Seed(ItemID id_, std::string name_, Identifier crop_tilename, MoneyCount base_price, ItemCount max_count):
		Plantable(id_, std::move(name_), base_price, max_count), cropTilename(std::move(crop_tilename)) {}

	bool Seed::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		auto &realm = *place.realm;
		auto &tileset = realm.getTileset();

		if (auto tile = realm.tryTile(Layer::Terrain, place.position); tile && tileset.isInCategory(*tile, "base:category/farmland"_id)) {
			if (auto submerged = realm.tryTile(Layer::Submerged, place.position); !submerged || *submerged == tileset.getEmptyID()) {
				const InventoryPtr inventory = place.player->getInventory(0);
				auto inventory_lock = inventory->uniqueLock();
				return plant(inventory, slot, stack, place, Layer::Submerged);
			}
		}

		return false;
	}

	bool Seed::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, DragAction) {
		return use(slot, stack, place, modifiers, offsets);
	}

	bool Seed::plant(InventoryPtr inventory, Slot slot, const ItemStackPtr &stack, const Place &place, Layer layer) {
		if (stack->count == 0) {
			inventory->erase(slot);
			inventory->notifyOwner({});
			return false;
		}

		place.set(layer, cropTilename);

		inventory->decrease(stack, slot, 1, false);

		return true;
	}
}
