#pragma once

#include "tileentity/DirectedTileEntity.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"

namespace Game3 {
	class Pump: public FluidHoldingTileEntity, public EnergeticTileEntity, public DirectedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/pump"}; }

			constexpr static EnergyAmount ENERGY_CAPACITY = 16'000;
			constexpr static double ENERGY_PER_UNIT = 0.5;

			FluidAmount extractionRate = 250;

			FluidAmount getMaxLevel(FluidID) override;

			std::string getName() const override { return "Pump"; }

			void tick(const TickArgs &) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		protected:
			std::string getDirectedTileBase() const override { return "base:tile/pump_"; }

		private:
			Pump();
			Pump(Identifier tile_id, Position);
			Pump(Position);

		friend class TileEntity;
	};
}
