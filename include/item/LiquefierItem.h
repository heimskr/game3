#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Liquefier.h"

namespace Game3 {
	class LiquefierItem: public TileEntityItem<Liquefier> {
		public:
			using TileEntityItem<Liquefier>::TileEntityItem;
	};
}
