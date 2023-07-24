#pragma once

#include "game/HasInventory.h"
#include "item/Item.h"
#include "tileentity/TileEntity.h"

#include <optional>

namespace Game3 {
	class InventoriedTileEntity: public TileEntity, public HasInventory {
		public:
			using TileEntity::TileEntity;

			virtual bool canInsertItem(const ItemStack &, Direction);
			virtual std::optional<ItemStack> extractItem(Direction, bool remove);

			virtual bool empty() const;

			/** Server-side only. */
			void setInventory(Slot slot_count);

			void inventoryUpdated() override;
			std::shared_ptr<Agent> getSharedAgent() final;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
	};
}
