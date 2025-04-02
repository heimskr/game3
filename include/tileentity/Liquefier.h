#pragma once

#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Liquefier: public FluidHoldingTileEntity, public EnergeticTileEntity, public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/liquefier"}; }

			FluidAmount getMaxLevel(FluidID) override;

			std::string getName() const override { return "Liquefier"; }

			void init(Game &) override;
			void tick(const TickArgs &) override;
			void toJSON(boost::json::value &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			Liquefier();
			Liquefier(Identifier tile_id, Position);
			Liquefier(Position);

		friend class TileEntity;
	};
}
