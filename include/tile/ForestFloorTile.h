#pragma once

#include "tile/Tile.h"

namespace Game3 {
	class ForestFloorTile: public Tile {
		public:
			static Identifier ID() { return {"base", "tile/forest_floor"}; }

			ForestFloorTile();

			void randomTick(const Place &) override;
			bool interact(const Place &, Layer, const ItemStackPtr &, Hand) override;
	};
}
