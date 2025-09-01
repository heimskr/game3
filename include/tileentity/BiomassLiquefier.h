#pragma once

#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class BiomassLiquefier: public FluidHoldingTileEntity, public InventoriedTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/biomass_liquefier"}; }

			size_t getMaxFluidTypes() const override;
			FluidAmount getMaxLevel(FluidID) override;

			std::string getName() const override { return "Biomass Liquefier"; }

			void init(Game &) override;
			void tick(const TickArgs &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void toJSON(boost::json::value &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, BasicBuffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			BiomassLiquefier();
			BiomassLiquefier(Identifier tile_id, Position);
			BiomassLiquefier(Position);

		friend class TileEntity;
	};
}
