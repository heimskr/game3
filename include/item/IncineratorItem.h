#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Incinerator.h"

namespace Game3 {
	class IncineratorItem: public TileEntityItem<Incinerator> {
		public:
			using TileEntityItem<Incinerator>::TileEntityItem;
	};
}
