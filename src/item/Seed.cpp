#include "Log.h"
#include "Position.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/Seed.h"
#include "realm/Realm.h"

namespace Game3 {
	Seed::Seed(ItemID id_, std::string name_, Identifier crop_tilename, MoneyCount base_price, ItemCount max_count):
		Item(id_, std::move(name_), base_price, max_count), cropTilename(std::move(crop_tilename)) {}

	bool Seed::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		auto &realm = *place.realm;
		auto &tileset = realm.getTileset();

		if (auto tile = realm.tryTile(Layer::Terrain, place.position); tile && tileset.isInCategory(*tile, "base:category/farmland"_id)) {
			if (auto submerged = realm.tryTile(Layer::Submerged, place.position); !submerged || *submerged == tileset.getEmptyID()) {
				const InventoryPtr inventory = place.player->getInventory();
				if (--stack.count == 0)
					inventory->erase(slot);
				else
					inventory->notifyOwner();
				place.set(Layer::Submerged, cropTilename);
				return true;
			}
		}

		return false;
	}
}
