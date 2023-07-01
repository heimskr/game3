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
			place.realm->updateNeighbors(place.position);



			// const TileID march_result = march4([&](int8_t march_row_offset, int8_t march_column_offset) -> bool {
			// 	const Position march_position = place.position + Position(march_row_offset, march_column_offset);
			// 	return tileset.isInCategory(tileset[place.realm->tileProvider.copyTile(Layer::ItemPipes, march_position, TileProvider::TileMode::ReturnEmpty)], "base:category/item_pipes"_id);
			// });

			// const TileID marched = tileset[tileset.getMarchBase("base:category/item_pipes"_id)] + (march_result / 7) * tileset.columnCount(place.getGame()) + march_result % 7;
			// INFO("marched[" << marched << "]");
			// place.set(Layer::ItemPipes, marched);
			// place.realm->reupload(Layer::ItemPipes);

			return true;
		}

		return false;
	}
}
