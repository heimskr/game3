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

			virtual bool canInsertFluid(FluidStack, Direction);
			/** Returns the amount not added. */
			virtual FluidAmount addFluid(FluidStack, Direction);
			virtual std::optional<FluidStack> extractFluid(Direction, bool remove, FluidAmount max_amount);
			virtual std::optional<FluidStack> extractFluid(Direction, bool remove);

			/** Server-side only. */
			void setFluidLevels(HasFluids::Map);

			void fluidsUpdated() override;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			using HasFluids::canInsertFluid;
			using HasFluids::addFluid;
	};
}
