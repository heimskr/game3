#pragma once

#include "tileentity/EnergeticTileEntity.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Dissolver: public InventoriedTileEntity, public EnergeticTileEntity {
		public:
			static Identifier ID() { return {"base", "te/dissolver"}; }

			bool mayInsertItem(const ItemStack &, Direction, Slot) override;
			bool mayExtractItem(Direction, Slot) override;
			EnergyAmount getEnergyCapacity() override;

			std::string getName() const override { return "Dissolver"; }

			void init(Game &) override;
			void tick(Game &, float) override;
			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

			Game & getGame() const final;

		private:
			float accumulatedTime = 0.f;
			Slot currentSlot = 0;

			Dissolver();
			Dissolver(Identifier tile_id, Position);
			Dissolver(Position);

			bool dissolve();

			friend class TileEntity;
	};
}
