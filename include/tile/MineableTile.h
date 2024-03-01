#pragma once

#include "item/Item.h"
#include "tile/Tile.h"

namespace Game3 {
	class MineableTile: public Tile {
		private:
			ItemStackPtr stack;
			bool consumable = false;

		public:
			MineableTile(Identifier, ItemStackPtr, bool consumable_);

			bool interact(const Place &, Layer, const ItemStackPtr &, Hand) override;
	};
}
