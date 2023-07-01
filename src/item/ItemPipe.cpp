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

		auto item_pipe = realm.tryTile(Layer::ItemPipes, place.position);

		if (!item_pipe || *item_pipe == tileset.getEmptyID()) {
			if (--stack.count == 0)
				place.player->inventory->erase(slot);
			else
				place.player->inventory->notifyOwner();
			place.set(Layer::ItemPipes, "base:tile/item_pipe"_id);
			return true;
		}

		const auto &tilename = tileset[*item_pipe];

		if (tileset.isInCategory(tilename, "base:category/item_pipes_normal"_id)) {
			Identifier alt = tilename;
			alt.name += "_alt";
			place.set(Layer::ItemPipes, alt);
			return true;
		}

		if (tileset.isInCategory(tilename, "base:category/item_pipes_alt"_id)) {
			Identifier normal = tilename;
			// Remove "_alt" from the end
			normal.name.pop_back();
			normal.name.pop_back();
			normal.name.pop_back();
			normal.name.pop_back();
			place.set(Layer::ItemPipes, normal);
			return true;
		}

		return false;
	}
}
