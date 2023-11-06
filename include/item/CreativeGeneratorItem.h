#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/CreativeGenerator.h"

namespace Game3 {
	class CreativeGeneratorItem: public TileEntityItem<CreativeGenerator> {
		public:
			using TileEntityItem<CreativeGenerator>::TileEntityItem;
	};
}
