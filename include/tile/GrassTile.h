#pragma once

#include "tile/Tile.h"

namespace Game3 {
	class GrassTile: public Tile {
		public:
			static Identifier ID() { return {"base", "tile/grass"}; }

			GrassTile();

			void randomTick(const Place &) override;
			bool interact(const Place &, Layer, const ItemStackPtr &used_item, Hand) override;
	};
}
