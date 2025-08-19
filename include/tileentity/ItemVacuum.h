#pragma once

#include "mixin/HasRadius.h"
#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class ItemVacuum: public InventoriedTileEntity, public EnergeticTileEntity, public HasRadius {
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
			bool setField(uint32_t field_name, Buffer &field_value, const PlayerPtr &updater) override;
			void broadcast(bool force) override;

			GamePtr getGame() const final;

		private:
			ItemVacuum();
			ItemVacuum(Identifier tile_id, Position);
			ItemVacuum(Position);

			bool scan();

		friend class TileEntity;
	};
}
