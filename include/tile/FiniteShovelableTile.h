#pragma once

#include "tile/Tile.h"

namespace Game3 {
	class FiniteShovelableTile: public Tile {
		public:
			Identifier itemID;

			FiniteShovelableTile(Identifier identifier, Identifier itemID);

			bool interact(const Place &, Layer, const ItemStackPtr &used_item, Hand) override;
	};
}
