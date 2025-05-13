#pragma once

#include "tile/Tile.h"

namespace Game3 {
	class AshTile: public Tile {
		public:
			static Identifier ID() { return {"base", "tile/ash"}; }

			AshTile();

			bool interact(const Place &, Layer, const ItemStackPtr &used_item, Hand) override;
	};
}
