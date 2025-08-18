#pragma once

#include "chemskr/Chemskr.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class ItemVacuum: public InventoriedTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/item_vacuum"}; }

			EnergyAmount getEnergyCapacity() override;

			std::string getName() const override { return "Item Vacuum"; }

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
			uint64_t radius = 2;

			ItemVacuum();
			ItemVacuum(Identifier tile_id, Position);
			ItemVacuum(Position);

			bool scan();

		friend class TileEntity;
	};
}
