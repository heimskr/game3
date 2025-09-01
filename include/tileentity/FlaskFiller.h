#pragma once

#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class FlaskFiller: public FluidHoldingTileEntity, public EnergeticTileEntity, public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/flask_filler"}; }

			FluidAmount getMaxLevel(FluidID) override;

			std::string getName() const override { return "Flask Filler"; }

			void init(Game &) override;
			void tick(const TickArgs &) override;
			void toJSON(boost::json::value &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, BasicBuffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			FlaskFiller();
			FlaskFiller(Identifier tile_id, Position);
			FlaskFiller(Position);

		friend class TileEntity;
	};
}
