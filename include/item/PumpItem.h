#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Pump.h"

namespace Game3 {
	class PumpItem: public TileEntityItem<Pump> {
		public:
			using TileEntityItem<Pump>::TileEntityItem;
	};
}
