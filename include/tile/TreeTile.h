#pragma once

#include "tile/CropTile.h"

namespace Game3 {
	class TreeTile: public CropTile {
		public:
			static Identifier ID() { return {"base", "tile/tree"}; }

			TreeTile(std::shared_ptr<Crop>);

			bool interact(const Place &, Layer) override;
	};
}
