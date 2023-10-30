#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Combiner.h"

namespace Game3 {
	class CombinerItem: public TileEntityItem<Combiner> {
		public:
			using TileEntityItem<Combiner>::TileEntityItem;
	};
}
