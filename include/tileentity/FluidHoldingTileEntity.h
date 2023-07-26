#pragma once

#include "game/HasFluids.h"
#include "item/Item.h"
#include "tileentity/TileEntity.h"

#include <optional>

namespace Game3 {
	/**
	 * This class inherits TileEntity *virtually*. It doesn't call any TileEntity methods itself.
	 * Deriving classes must remember to do so in the encode and decode methods.
	 */
	class FluidHoldingTileEntity: public virtual TileEntity, public HasFluids {
		public:
			FluidHoldingTileEntity(HasFluids::Map = {});

			// virtual bool canInsertItem(const ItemStack &, Direction);
			// virtual std::optional<ItemStack> extractItem(Direction, bool remove);

			virtual bool empty();

			/** Server-side only. */
			void setFluidLevels(HasFluids::Map);

			void fluidsUpdated() override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
	};
}
