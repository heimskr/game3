#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/ChemicalReactor.h"

namespace Game3 {
	class ChemicalReactorItem: public TileEntityItem<ChemicalReactor> {
		public:
			using TileEntityItem<ChemicalReactor>::TileEntityItem;
	};
}
