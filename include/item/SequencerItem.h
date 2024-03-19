#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Recombinator.h"

namespace Game3 {
	class RecombinatorItem: public TileEntityItem<Recombinator> {
		public:
			using TileEntityItem<Recombinator>::TileEntityItem;
	};
}
