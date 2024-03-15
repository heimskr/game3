#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Mutator.h"

namespace Game3 {
	class MutatorItem: public TileEntityItem<Mutator> {
		public:
			using TileEntityItem<Mutator>::TileEntityItem;
	};
}
