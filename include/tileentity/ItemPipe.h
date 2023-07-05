#pragma once

#include "Directions.h"
#include "Types.h"
#include "container/DirectionalContainer.h"
#include "tileentity/Pipe.h"

namespace Game3 {
	class ItemPipe: public Pipe {
		public:
			static Identifier ID() { return {"base", "te/item_pipe"}; }

		protected:
			ItemPipe(Position = {});

			friend class TileEntity;
	};
}
