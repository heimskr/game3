#pragma once

#ifdef GAME3_ENABLE_SCRIPTING
#include "item/TileEntityItem.h"
#include "tileentity/Computer.h"

namespace Game3 {
	class ComputerItem: public TileEntityItem<Computer> {
		public:
			using TileEntityItem<Computer>::TileEntityItem;
	};
}
#endif
