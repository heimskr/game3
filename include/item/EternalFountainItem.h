#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/EternalFountain.h"

namespace Game3 {
	class EternalFountainItem: public TileEntityItem<EternalFountain> {
		public:
			using TileEntityItem<EternalFountain>::TileEntityItem;
	};
}
