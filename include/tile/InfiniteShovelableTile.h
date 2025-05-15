#pragma once

#include "tile/Tile.h"

namespace Game3 {
	class InfiniteShovelableTile: public Tile {
		public:
			Identifier itemID;

			InfiniteShovelableTile(Identifier identifier, Identifier itemID);

			bool interact(const Place &, Layer, const ItemStackPtr &used_item, Hand) override;
	};
}
