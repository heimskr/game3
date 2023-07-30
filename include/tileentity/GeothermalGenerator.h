#pragma once

#include "Texture.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"

namespace Game3 {
	class GeothermalGenerator: public FluidHoldingTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/geothermal_generator"}; }

			constexpr static float PERIOD = 0.25;

			FluidAmount getMaxLevel(FluidID) const override;
			EnergyAmount getEnergyCapacity() override;

			void tick(Game &, float) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast() override;

			Game & getGame() const final;

		private:
			float accumulatedTime = 0.f;

			GeothermalGenerator() = default;
			GeothermalGenerator(Identifier tile_id, Position);
			GeothermalGenerator(Position);

			friend class TileEntity;
	};
}
