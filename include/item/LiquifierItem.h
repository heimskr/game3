#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Liquifier.h"

namespace Game3 {
	class LiquifierItem: public TileEntityItem<Liquifier> {
		public:
			using TileEntityItem<Liquifier>::TileEntityItem;
	};
}
