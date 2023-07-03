#include "Extractors.h"
#include "Log.h"
#include "Position.h"
#include "Quadrant.h"
#include "Tileset.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "item/ItemPipe.h"
#include "realm/Realm.h"


namespace Game3 {
	static Extractors getExtractors(Index corner, Index sheet_index, Index column_count) {
		if (sheet_index == 0)
			return {};

		const auto column = (sheet_index - corner) % column_count;
		// We might be able to apply the distributive property here, but I'm not sure how it would work with rounding and I don't feel like thinking for maybe 20 seconds.
		const auto row = sheet_index / column_count - corner / column_count;
		return Extractors(column + row * 8);
	}

	ItemPipe::ItemPipe(MoneyCount base_price):
		Item("base:item/item_pipe"_id, "Item Pipe", base_price, 64) {}

	bool ItemPipe::use(Slot slot, ItemStack &stack, const Place &place, Modifiers modifiers, std::pair<float, float> offsets) {
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

		const auto [x, y] = offsets;

		if (modifiers.onlyShift() && tileset.isInCategory(tilename, "base:category/item_pipes"_id)) {
			place.player->give(ItemStack(place.getGame(), shared_from_this(), 1));
			place.set(Layer::ItemPipes, 0);
			place.set(Layer::ItemExtractors, 0);
			return true;
		}

		const size_t column_count = tileset.columnCount(place.getGame());

		// Hold ctrl to toggle extractors.
		if (modifiers.onlyCtrl()) {
			if (auto sheet_index = place.get(Layer::ItemExtractors)) {
				const TileID corner = tileset["base:tile/extractor_"_id];
				Extractors extractors = getExtractors(corner, *sheet_index, column_count);
				// Detect the center, otherwise use quadrants.
				if (0.375 <= x && x <= 0.625 && 0.375 <= y && y <= 0.625)
					extractors.toggleMiddle();
				else
					extractors.toggleQuadrant(getQuadrant(x, y));
				const auto [x_offset, y_offset] = extractors.getOffsets();
				const TileID new_tile = corner + x_offset + y_offset * column_count;
				place.set(Layer::ItemExtractors, new_tile);
			}
		} else {
			const TileID corner = tileset["base:tile/item_pipe_se"_id];
			const Quadrant quadrant = getQuadrant(x, y);
			int8_t march_index = (tileset[tilename] - corner) % column_count + 7 * (tileset[tilename] / column_count - corner / column_count);
			march_index = toggleQuadrant(march_index, quadrant);
			const TileID new_tile = corner + column_count * (march_index / 7) + march_index % 7;
			place.realm->setTile(Layer::ItemPipes, place.position, new_tile, false, false);
			return true;
		}

		return false;
	}
}
