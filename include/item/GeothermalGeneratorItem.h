#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/GeothermalGenerator.h"

namespace Game3 {
	class GeothermalGeneratorItem: public TileEntityItem<GeothermalGenerator> {
		public:
			using TileEntityItem<GeothermalGenerator>::TileEntityItem;
	};
}
