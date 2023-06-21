#pragma once

#include "tile/Tile.h"

namespace Game3 {
	class Crop;

	class CropTile: public Tile {
		public:
			static Identifier ID() { return {"base", "tile/crop"}; }

			std::shared_ptr<Crop> crop;

			CropTile(std::shared_ptr<Crop>);

			void randomTick(const Place &) override;
	};
}
