#pragma once

#include "game/HasInventory.h"
#include "item/Item.h"
#include "tileentity/TileEntity.h"

#include <optional>

namespace Game3 {
	/**
	 * This class inherits TileEntity *virtually*. It doesn't call any TileEntity methods itself.
	 * Deriving classes must remember to do so in the encode and decode methods.
	 */
	class InventoriedTileEntity: public virtual TileEntity, public HasInventory {
		public:
			InventoriedTileEntity(std::shared_ptr<Inventory> = nullptr);

			virtual bool canInsertItem(const ItemStack &, Direction);
			virtual std::optional<ItemStack> extractItem(Direction, bool remove);

			virtual bool empty() const;

			/** Server-side only. */
			void setInventory(Slot slot_count);

			void inventoryUpdated() override;
			std::shared_ptr<Agent> getSharedAgent() final;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
	};
}
