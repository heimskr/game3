#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Dissolver.h"

namespace Game3 {
	class DissolverItem: public TileEntityItem<Dissolver> {
		public:
			using TileEntityItem<Dissolver>::TileEntityItem;
	};
}
