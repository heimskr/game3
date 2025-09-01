#pragma once

#include "tileentity/FluidHoldingTileEntity.h"

namespace Game3 {
	class Tank: public FluidHoldingTileEntity {
		public:
			static Identifier ID() { return {"base", "te/tank"}; }

			FluidAmount getMaxLevel(FluidID) override;

			std::string getName() const override { return "Tank"; }

			void toJSON(boost::json::value &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, BasicBuffer &) override;

			GamePtr getGame() const final;

		private:
			Tank() = default;
			Tank(Identifier tile_id, Position);
			Tank(Position);

		friend class TileEntity;
	};
}
