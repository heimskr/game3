#pragma once

#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Incinerator: public FluidHoldingTileEntity, public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/incinerator"}; }

			size_t getMaxFluidTypes() const override;
			FluidAmount getMaxLevel(FluidID) override;

			void init(Game &) override;
			void tick(Game &, float) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *) override;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			Game & getGame() const final;

		private:
			float accumulatedTime = 0.f;

			Incinerator() = default;
			Incinerator(Identifier tile_id, Position);
			Incinerator(Position);

			friend class TileEntity;
	};
}
