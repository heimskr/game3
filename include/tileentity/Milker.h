#pragma once

#include "mixin/HasRadius.h"
#include "tileentity/DirectedTileEntity.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"

namespace Game3 {
	class Milker: public FluidHoldingTileEntity, public EnergeticTileEntity, public DirectedTileEntity, public HasRadius {
		public:
			static Identifier ID() { return {"base", "te/milker"}; }

			constexpr static EnergyAmount ENERGY_CAPACITY = 16'000;
			constexpr static double ENERGY_PER_UNIT = 0.5;

			FluidAmount getMaxLevel(FluidID) override;

			std::string getName() const override { return "Milker"; }

			void tick(const TickArgs &) override;
			void toJSON(boost::json::value &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, BasicBuffer &) override;
			bool setField(uint32_t field_name, Buffer &field_value, const PlayerPtr &updater) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		protected:
			std::string getDirectedTileBase() const override { return "base:tile/milker_"; }

		private:
			Milker();
			Milker(Identifier tile_id, Position);
			Milker(Position);

		friend class TileEntity;
	};
}
