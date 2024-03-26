#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Disruptor.h"

namespace Game3 {
	class DisruptorItem: public TileEntityItem<Disruptor> {
		public:
			using TileEntityItem<Disruptor>::TileEntityItem;
	};
}
