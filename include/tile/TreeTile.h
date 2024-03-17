#pragma once

#include "data/Identifier.h"
#include "tile/CropTile.h"
#include "types/Types.h"

namespace Game3 {
	class TreeTile: public CropTile {
		public:
			static Identifier ID() { return {"base", "tile/tree"}; }

			TreeTile(std::shared_ptr<Crop>);

			bool interact(const Place &, Layer, const ItemStackPtr &, Hand) override;
	};
}
