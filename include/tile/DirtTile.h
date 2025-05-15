#pragma once

#include "tile/InfiniteShovelableTile.h"

namespace Game3 {
	class DirtTile: public InfiniteShovelableTile {
		public:
			static Identifier ID() { return {"base", "tile/dirt"}; }

			DirtTile();

			void randomTick(const Place &) override;
	};
}
