#include "Extractors.h"
#include "Log.h"
#include "Position.h"
#include "Quadrant.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "game/ServerGame.h"
#include "item/ItemPipeItem.h"
#include "realm/Realm.h"
#include "tileentity/ItemPipe.h"

namespace Game3 {
	static Extractors getExtractors(Index corner, Index sheet_index, Index column_count) {
		if (sheet_index == 0)
			return {};

		const auto column = (sheet_index - corner) % column_count;
		// We might be able to apply the distributive property here, but I'm not sure how it would work with rounding and I don't feel like thinking for maybe 20 seconds.
		const auto row = sheet_index / column_count - corner / column_count;
		return Extractors(column + row * 8);
	}

	ItemPipeItem::ItemPipeItem(MoneyCount base_price):
		Item("base:item/item_pipe"_id, "Item Pipe", base_price, 64) {}

	bool ItemPipeItem::use(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets) {
		auto &realm = *place.realm;

		auto tile_entity = realm.tileEntityAt(place.position);
		if (!tile_entity) {
			if (modifiers.onlyShift())
				return false;

			if (--stack.count == 0)
				place.player->inventory->erase(slot);
			else
				place.player->inventory->notifyOwner();

			auto pipe = TileEntity::create<ItemPipe>(realm.getGame(), place.position);
			if (place.realm->add(pipe))
				realm.getGame().toServer().tileEntitySpawned(pipe);

			return true;
		}

		auto pipe = std::dynamic_pointer_cast<Pipe>(tile_entity);
		if (!pipe)
			return false;

		if (modifiers.onlyShift()) {
			place.player->give(ItemStack(place.getGame(), shared_from_this(), 1));
			realm.queueDestruction(pipe);
			return true;
		}

		const auto [x, y] = offsets;
		const Quadrant quadrant = getQuadrant(x, y);

		// Hold ctrl to toggle extractors.
		if (modifiers.onlyCtrl()) {
			if (pipe->getDirections()[quadrant])
				pipe->getExtractors().toggle(quadrant);
		} else if (!pipe->getDirections().toggle(quadrant)) {
			pipe->getExtractors()[quadrant] = false;
		}

		pipe->increaseUpdateCounter();
		pipe->broadcast();
		return true;
	}

	// 	const size_t column_count = tileset.columnCount(place.getGame());

	// 	if (modifiers.onlyCtrl()) {
	// 	} else {
	// 		const TileID corner = tileset["base:tile/item_pipe_se"_id];
	// 		const Quadrant quadrant = getQuadrant(x, y);
	// 		int8_t march_index = (tileset[tilename] - corner) % column_count + 7 * (tileset[tilename] / column_count - corner / column_count);
	// 		march_index = toggleQuadrant(march_index, quadrant);
	// 		const TileID new_tile = corner + column_count * (march_index / 7) + march_index % 7;
	// 		place.realm->setTile(Layer::ItemPipes, place.position, new_tile, false, false);
	// 		return true;
	// 	}

	// 	return false;
	// }
}
