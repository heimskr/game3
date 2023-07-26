#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Tank.h"

namespace Game3 {
	class TankItem: public TileEntityItem<Tank> {
		public:
			using TileEntityItem<Tank>::TileEntityItem;
	};
}
