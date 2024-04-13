#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Lamp.h"

namespace Game3 {
	class LampItem: public TileEntityItem<Lamp> {
		public:
			using TileEntityItem<Lamp>::TileEntityItem;
	};
}
