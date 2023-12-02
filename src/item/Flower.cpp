#include "types/Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Flower.h"
#include "realm/Realm.h"

namespace Game3 {
	Flower::Flower(ItemID identifier_, std::string name_, Identifier tilename_, Identifier valid_ground, MoneyCount base_price, ItemCount max_count):
	Plantable(std::move(identifier_), std::move(name_), base_price, max_count),
	tilename(std::move(tilename_)),
	validGround(std::move(valid_ground)) {
		attributes.insert("base:attribute/plantable"_id);
		attributes.insert("base:attribute/flower"_id);
	}

	bool Flower::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>, Hand) {
		auto &realm = *place.realm;
		const auto &position = place.position;
		const auto &tileset = realm.getTileset();

		if (realm.isPathable(position) && realm.middleEmpty(position))
			if (!validGround || tileset.isInCategory(tileset[realm.getTile(Layer::Terrain, position)], validGround))
				return plant(place.player->getInventory(0), slot, stack, place);

		return false;
	}

	bool Flower::plant(InventoryPtr inventory, Slot slot, ItemStack &stack, const Place &place) {
		if (stack.count == 0) {
			inventory->erase(slot);
			inventory->notifyOwner();
			return false;
		}

		place.set(Layer::Submerged, tilename);
		if (--stack.count == 0)
			inventory->erase(slot);
		inventory->notifyOwner();
		return true;
	}
}
