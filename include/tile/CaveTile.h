#pragma once

#include "tile/MineableTile.h"

namespace Game3 {
	class CaveTile: public MineableTile {
		private:
			ItemStack stack;
			Identifier floor;

		public:
			CaveTile(Identifier, ItemStack, Identifier floor_);

			bool interact(const Place &, Layer, ItemStack *) override;
	};
}
