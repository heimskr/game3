#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Autofarmer.h"

namespace Game3 {
	class AutofarmerItem: public TileEntityItem<Autofarmer> {
		public:
			using TileEntityItem<Autofarmer>::TileEntityItem;
	};
}
