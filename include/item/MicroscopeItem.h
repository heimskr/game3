#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Microscope.h"

namespace Game3 {
	class MicroscopeItem: public TileEntityItem<Microscope> {
		public:
			using TileEntityItem<Microscope>::TileEntityItem;
	};
}
