#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/BiomassLiquefier.h"

namespace Game3 {
	class BiomassLiquefierItem: public TileEntityItem<BiomassLiquefier> {
		public:
			using TileEntityItem<BiomassLiquefier>::TileEntityItem;
	};
}
