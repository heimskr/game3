#pragma once

#include "tile/Tile.h"

namespace Game3 {
	class TorchTile: public Tile {
		public:
			static Identifier ID() { return {"base", "tile/torch"}; }

			TorchTile();

			void renderStaticLighting(const Place &, Layer, const RendererSet &) override;
	};
}
