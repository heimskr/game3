#pragma once

#include "tile/Tile.h"

namespace Game3 {
	class SnowTile: public Tile {
		public:
			static Identifier ID() { return {"base", "tile/snow"}; }

			SnowTile();

			std::optional<FluidTile> yieldFluid(const Place &) final;

		private:
			std::optional<FluidID> cachedFluidID;
	};
}
