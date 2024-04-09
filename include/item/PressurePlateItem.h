#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/PressurePlate.h"

namespace Game3 {
	class PressurePlateItem: public TileEntityItem<PressurePlate> {
		public:
			using TileEntityItem<PressurePlate>::TileEntityItem;
	};
}
