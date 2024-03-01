#pragma once

#include "tile/MineableTile.h"

namespace Game3 {
	class CaveTile: public MineableTile {
		private:
			ItemStackPtr stack;
			Identifier floor;

		public:
			CaveTile(Identifier, ItemStackPtr, Identifier floor_);

			bool interact(const Place &, Layer, const ItemStackPtr &, Hand) override;
	};
}
