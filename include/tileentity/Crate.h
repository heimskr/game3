#pragma once

#include "game/Inventory.h"
#include "threading/Lockable.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	class Crate: public InventoriedTileEntity {
		public:
			static Identifier ID() { return {"base", "te/crate"}; }

			std::string name;

			std::string getName() const override;

			bool mayInsertItem(const ItemStack &, Direction, Slot) override;
			bool mayExtractItem(Direction, Slot) override;
			bool canInsertItem(const ItemStack &, Direction, Slot) override;
			bool canExtractItem(Direction, Slot) override;
			std::optional<ItemStack> extractItem(Direction, bool remove, Slot) override;
			bool insertItem(const ItemStack &, Direction, std::optional<ItemStack> *) override;
			ItemCount itemsInsertable(const ItemStack &, Direction, Slot) override;
			void iterateExtractableItems(Direction, const std::function<bool(const ItemStack &, Slot)> &) override;
			bool empty() const override;

			void toJSON(nlohmann::json &) const override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void inventoryUpdated() override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;

		protected:
			Crate() = default;
			Crate(Identifier tile_id, const Position &, std::string name_);
			Crate(const Position &);

		private:
			Lockable<std::optional<ItemStack>> storedStack;

			void setInventoryStack();
			void absorbStackFromInventory();

		friend class TileEntity;
	};
}
