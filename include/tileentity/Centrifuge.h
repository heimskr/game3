#pragma once

#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Centrifuge: public FluidHoldingTileEntity, public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/centrifuge"}; }

			constexpr static float PERIOD = 0.25;

			size_t getMaxFluidTypes() const override;
			FluidAmount getMaxLevel(FluidID) override;

			void init(Game &) override;
			void tick(Game &, float) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			Game & getGame() const final;

		private:
			float accumulatedTime = 0.f;

			Centrifuge() = default;
			Centrifuge(Identifier tile_id, Position);
			Centrifuge(Position);

			friend class TileEntity;
	};
}
