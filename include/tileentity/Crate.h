#pragma once

#include "game/Inventory.h"
#include "threading/Lockable.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Crate: public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/crate"}; }

			~Crate() override = default;

			Identifier itemName;
			std::string name;

			std::string getName() const override;

			ItemCount itemsInsertable(const ItemStack &, Direction, Slot) override;

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) override;

			void setInventory(Slot slot_count) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			Crate() = default;
			Crate(Identifier tile_id, const Position &, Identifier item_name, std::string name_);
			Crate(const Position &);

		friend class TileEntity;
	};
}
