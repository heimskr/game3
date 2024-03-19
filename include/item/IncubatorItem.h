#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Incubator.h"

namespace Game3 {
	class IncubatorItem: public TileEntityItem<Incubator> {
		public:
			using TileEntityItem<Incubator>::TileEntityItem;
	};
}
