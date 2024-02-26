#pragma once

#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Incinerator: public FluidHoldingTileEntity, public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/incinerator"}; }

			size_t getMaxFluidTypes() const override;
			FluidAmount getMaxLevel(FluidID) override;

			std::string getName() const override { return "Incinerator"; }

			void init(Game &) override;
			void tick(const TickArgs &) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			Incinerator() = default;
			Incinerator(Identifier tile_id, Position);
			Incinerator(Position);

			friend class TileEntity;
	};
}
