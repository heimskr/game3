#pragma once

#include "item/Item.h"
#include "tile/Tile.h"

namespace Game3 {
	class FenceGateTile: public Tile {
		public:
			using Tile::Tile;

			bool interact(const Place &, Layer, const ItemStackPtr &, Hand) override;
			bool update(const Place &, Layer) override;
	};
}
