#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Autocrafter.h"

namespace Game3 {
	class AutocrafterItem: public TileEntityItem<Autocrafter> {
		public:
			using TileEntityItem<Autocrafter>::TileEntityItem;
	};
}
