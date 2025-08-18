#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/ItemVacuum.h"

namespace Game3 {
	class ItemVacuumItem: public TileEntityItem<ItemVacuum> {
		public:
			using TileEntityItem<ItemVacuum>::TileEntityItem;
	};
}
