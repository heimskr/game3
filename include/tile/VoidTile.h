#pragma once

#include "item/Item.h"
#include "tile/Tile.h"

namespace Game3 {
	class VoidTile: public Tile {
		public:
			VoidTile();

			static Identifier ID() { return {"base", "tile/void"}; }

			bool interact(const Place &, Layer, const ItemStackPtr &, Hand) override;
	};
}
