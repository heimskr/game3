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

			/** Doesn't lock the inventory. */
			virtual bool canInsertItem(const ItemStack &, Direction);
			/** Doesn't lock the inventory. */
			virtual std::optional<ItemStack> extractItem(Direction, bool remove);

			/** Doesn't lock the inventory. */
			virtual bool empty() const;

			/** Server-side only. Doesn't lock the inventory. */
			void setInventory(Slot slot_count);

			void inventoryUpdated() override;
			std::shared_ptr<Agent> getSharedAgent() final;
			void absorbJSON(Game &, const nlohmann::json &) override;

			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
	};
}
