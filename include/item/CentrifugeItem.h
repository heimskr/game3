#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Centrifuge.h"

namespace Game3 {
	class CentrifugeItem: public TileEntityItem<Centrifuge> {
		public:
			using TileEntityItem<Centrifuge>::TileEntityItem;
	};
}
