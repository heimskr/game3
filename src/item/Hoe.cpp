#include "util/Log.h"
#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Hoe.h"
#include "realm/Realm.h"

namespace Game3 {
	Hoe::Hoe(ItemID id_, std::string name_, MoneyCount base_price, Durability max_durability):
		Tool(id_, std::move(name_), base_price, 0.f, max_durability, "base:attribute/hoe"_id) {}

	bool Hoe::use(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers, std::pair<float, float>) {
		Realm &realm = *place.realm;
		Tileset &tileset = realm.getTileset();

		if (std::optional<TileID> tile = realm.tryTile(Layer::Soil, place.position); tile && tileset.isInCategory(*tile, "base:category/tillable"_id)) {
			if (stack->reduceDurability()) {
				place.player->getInventory(0)->erase(slot);
			}
			place.player->getInventory(0)->notifyOwner({});
			place.set(Layer::Soil, "base:tile/farmland"_id);
			return true;
		}

		return false;
	}

	bool Hoe::drag(Slot slot, const ItemStackPtr &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets, DragAction) {
		return use(slot, stack, place, modifiers, offsets);
	}
}
