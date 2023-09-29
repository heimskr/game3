#pragma once

#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"

namespace Game3 {
	class Pump: public FluidHoldingTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/pump"}; }

			constexpr static EnergyAmount ENERGY_CAPACITY = 16'000;
			constexpr static double ENERGY_PER_UNIT = 0.5;
			constexpr static float PERIOD = 0.25;

			FluidAmount extractionRate = 250;

			inline Direction getDirection() const { return pumpDirection; }
			void setDirection(Direction);

			FluidAmount getMaxLevel(FluidID) override;

			std::string getName() const override { return "Pump"; }

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
			Direction pumpDirection = Direction::Down;

			Pump();
			Pump(Identifier tile_id, Position);
			Pump(Position);

			friend class TileEntity;
	};
}
