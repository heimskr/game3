#include "Log.h"
#include "Position.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Hoe.h"
#include "realm/Realm.h"

namespace Game3 {
	Hoe::Hoe(ItemID id_, std::string name_, MoneyCount base_price, Durability max_durability):
		Tool(id_, std::move(name_), base_price, 0.f, max_durability, "base:attribute/hoe"_id) {}

	bool Hoe::use(Slot slot, ItemStack &stack, const Place &place, Modifiers) {
		auto &realm = *place.realm;
		auto &tileset = realm.getTileset();

		if (auto tile = realm.tryTile(Layer::Terrain, place.position); tile && tileset.isInCategory(*tile, "base:category/tillable"_id)) {
			if (stack.reduceDurability())
				place.player->inventory->erase(slot);
			place.set(Layer::Terrain, "base:tile/farmland"_id);
			return true;
		}

		return false;
	}
}
