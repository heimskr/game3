#pragma once

#include "entity/LivingEntity.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/FluidHoldingTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Incubator: public FluidHoldingTileEntity, public InventoriedTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/incubator"}; }

			size_t getMaxFluidTypes() const override;
			FluidAmount getMaxLevel(FluidID) override;

			std::string getName() const override { return "Incubator"; }

			void init(Game &) override;
			void tick(const TickArgs &) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void toJSON(boost::json::value &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			std::optional<FluidID> biomassID;

			Incubator();
			Incubator(Identifier tile_id, Position);
			Incubator(Position);

			static LivingEntityPtr makeEntity(const GamePtr &, const boost::json::value &genes);

		friend class TileEntity;
	};
}
