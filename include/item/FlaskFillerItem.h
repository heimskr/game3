#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/FlaskFiller.h"

namespace Game3 {
	class FlaskFillerItem: public TileEntityItem<FlaskFiller> {
		public:
			using TileEntityItem<FlaskFiller>::TileEntityItem;
	};
}
