#include "Log.h"
#include "Position.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/ItemPipe.h"
#include "realm/Realm.h"

#include "MarchingSquares.h"

namespace Game3 {
	ItemPipe::ItemPipe(MoneyCount base_price):
		Item("base:item/item_pipe"_id, "Item Pipe", base_price, 64) {}

	bool ItemPipe::use(Slot slot, ItemStack &stack, const Place &place, Modifiers) {
		auto &realm = *place.realm;
		auto &tileset = realm.getTileset();

		if (auto item_pipe = realm.tryTile(Layer::ItemPipes, place.position); !item_pipe || *item_pipe == tileset.getEmptyID()) {
			if (--stack.count == 0)
				place.player->inventory->erase(slot);
			else
				place.player->inventory->notifyOwner();
			place.set(Layer::ItemPipes, "base:tile/item_pipe"_id);
			return true;
		}

		return false;
	}
}
