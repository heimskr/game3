#pragma once

#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class EternalFountain: public FluidHoldingTileEntity, public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/eternal_fountain"}; }

			size_t getMaxFluidTypes() const override;
			FluidAmount getMaxLevel(FluidID) override;

			std::string getName() const override { return "Eternal Fountain"; }

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
			EternalFountain();
			EternalFountain(Identifier tileID, Position);
			EternalFountain(Position);

		friend class TileEntity;
	};
}
