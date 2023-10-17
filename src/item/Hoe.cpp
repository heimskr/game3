#include "Log.h"
#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Hoe.h"
#include "realm/Realm.h"

namespace Game3 {
	Hoe::Hoe(ItemID id_, std::string name_, MoneyCount base_price, Durability max_durability):
		Tool(id_, std::move(name_), base_price, 0.f, max_durability, "base:attribute/hoe"_id) {}

	bool Hoe::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		Realm &realm = *place.realm;
		Tileset &tileset = realm.getTileset();

		if (auto tile = realm.tryTile(Layer::Terrain, place.position); tile && tileset.isInCategory(*tile, "base:category/tillable"_id)) {
			if (stack.reduceDurability())
				place.player->getInventory()->erase(slot);
			place.player->getInventory()->notifyOwner();
			place.set(Layer::Terrain, "base:tile/farmland"_id);
			return true;
		}

		return false;
	}

	bool Hoe::drag(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers) {
		return use(slot, stack, place, modifiers, {0.f, 0.f});
	}
}
