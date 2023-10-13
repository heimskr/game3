#pragma once

#include "tile/Tile.h"

namespace Game3 {
	class DirtTile: public Tile {
		public:
			static Identifier ID() { return {"base", "tile/dirt"}; }

			DirtTile();

			void randomTick(const Place &) override;
	};
}
