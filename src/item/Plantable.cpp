#include "Position.h"
#include "graphics/Tileset.h"
#include "entity/Player.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Plantable.h"
#include "realm/Realm.h"

namespace Game3 {
	Plantable::Plantable(ItemID identifier_, std::string name_, Identifier tilename_, Identifier valid_ground, MoneyCount base_price, ItemCount max_count):
	Item(std::move(identifier_), std::move(name_), base_price, max_count),
	tilename(std::move(tilename_)),
	validGround(std::move(valid_ground)) {
		attributes.insert("base:attribute/plantable"_id);
	}

	bool Plantable::use(Slot slot, ItemStack &stack, const Place &place, Modifiers, std::pair<float, float>) {
		auto &realm = *place.realm;
		const auto &position = place.position;
		const auto &tileset = realm.getTileset();

		if (realm.isPathable(position) && realm.middleEmpty(position)) {
			if (!validGround || tileset.isInCategory(tileset[realm.getTile(Layer::Terrain, position)], validGround)) {
				realm.setTile(Layer::Submerged, position, tilename);
				realm.reupload(Layer::Submerged);
				const InventoryPtr inventory = place.player->getInventory();
				if (--stack.count == 0)
					inventory->erase(slot);
				inventory->notifyOwner();
				return true;
			}
		}

		return false;
	}
}
