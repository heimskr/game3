#pragma once

#include "tile/MineableTile.h"

namespace Game3 {
	class CaveTile: public MineableTile {
		public:
			CaveTile(Identifier identifier, ItemStackPtr stack, Layer floorLayer, Identifier floor);

			bool interact(const Place &, Layer, const ItemStackPtr &, Hand) override;
			bool damage(const Place &, Layer) override;

		private:
			Identifier floor;
			Layer floorLayer{};

			void reveal(const Place &) const;
	};
}
