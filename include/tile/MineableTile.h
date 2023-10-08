#pragma once

#include "item/Item.h"
#include "tile/Tile.h"

namespace Game3 {
	class MineableTile: public Tile {
		private:
			ItemStack stack;
			bool consumable = false;

		public:
			MineableTile(Identifier, ItemStack, bool consumable_);

			bool interact(const Place &, Layer) override;
	};
}
