#pragma once

#include "item/TileEntityItem.h"
#include "tileentity/Sequencer.h"

namespace Game3 {
	class SequencerItem: public TileEntityItem<Sequencer> {
		public:
			using TileEntityItem<Sequencer>::TileEntityItem;
	};
}
