#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Milker.h"

namespace Game3 {
	class MilkerItem: public TileEntityItem<Milker> {
		public:
			using TileEntityItem<Milker>::TileEntityItem;
	};
}
