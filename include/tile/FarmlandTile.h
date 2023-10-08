#pragma once

#include "tile/Tile.h"

namespace Game3 {
	class FarmlandTile: public Tile {
		public:
			static Identifier ID() { return {"base", "tile/farmland"}; }

			FarmlandTile();

			bool interact(const Place &, Layer) override;
	};
}
