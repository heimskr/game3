#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Computer.h"

namespace Game3 {
	class ComputerItem: public TileEntityItem<Computer> {
		public:
			using TileEntityItem<Computer>::TileEntityItem;
	};
}
