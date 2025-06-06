#pragma once

#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Dissolver: public InventoriedTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/dissolver"}; }

			bool mayInsertItem(const ItemStackPtr &, Direction, Slot) override;
			bool mayExtractItem(Direction, Slot) override;
			EnergyAmount getEnergyCapacity() override;

			std::string getName() const override { return "Dissolver"; }

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
			Slot currentSlot = 0;

			Dissolver();
			Dissolver(Identifier tile_id, Position);
			Dissolver(Position);

			bool dissolve();

		friend class TileEntity;
	};
}
