#pragma once

#include "tile/Tile.h"

namespace Game3 {
	class Item;

	class FoodTile: public Tile {
		public:
			FoodTile(Identifier tilename, Identifier food_item_name);

			bool interact(const Place &, Layer, const ItemStackPtr &, Hand) override;

		private:
			Identifier foodItemName;
	};
}
