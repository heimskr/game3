#pragma once

#include "tile/InfiniteShovelableTile.h"

namespace Game3 {
	class GrassTile: public InfiniteShovelableTile {
		public:
			static Identifier ID() { return {"base", "tile/grass"}; }

			GrassTile();
			GrassTile(Identifier tileID);

			void randomTick(const Place &) override;
			bool interact(const Place &, Layer, const ItemStackPtr &used_item, Hand) override;
	};
}
