#pragma once

#include "game/HasInventory.h"
#include "game/Observable.h"
#include "item/Item.h"
#include "tileentity/TileEntity.h"

#include <functional>
#include <optional>

namespace Game3 {
	class TileEntityPacket;

	/**
	 * This class inherits TileEntity *virtually*. It doesn't call any TileEntity methods itself.
	 * Deriving classes must remember to do so in the encode and decode methods.
	 * None of the methods here lock the inventory.
	 */
	class InventoriedTileEntity: public virtual TileEntity, public HasInventory, public Observable {
		public:
			InventoriedTileEntity(std::shared_ptr<Inventory> = nullptr);

			virtual bool mayInsertItem(const ItemStack &, Direction, Slot) { return true; }
			virtual bool mayInsertItem(const ItemStack &stack, Direction direction) { return mayInsertItem(stack, direction, -1); }
			virtual bool mayExtractItem(Direction, Slot) { return true; }
			virtual bool mayExtractItem(Direction direction) { return mayExtractItem(direction, -1); }
			virtual bool canInsertItem(const ItemStack &, Direction, Slot);
			virtual bool canExtractItem(Direction, Slot);
			/** Returns the extracted item. */
			virtual std::optional<ItemStack> extractItem(Direction, bool remove, Slot slot = -1);
			/** Returns whether the item was insertable at all. */
			virtual bool insertItem(const ItemStack &, Direction, std::optional<ItemStack> *);
			virtual ItemCount itemsInsertable(const ItemStack &, Direction, Slot);
			/** Iterates over each extractable item until there all have been iterated or the iteration function returns true. */
			virtual void iterateExtractableItems(Direction, const std::function<bool(const ItemStack &, Slot)> &);

			virtual bool empty() const;

			/** Server-side only. */
			void setInventory(Slot slot_count);

			void inventoryUpdated() override;
			std::shared_ptr<Agent> getSharedAgent() final;

			void addObserver(const std::shared_ptr<Player> &, bool silent) override;

			void absorbJSON(Game &, const nlohmann::json &) override;
			void encode(Game &, Buffer &) override;
			void decode(Game &, Buffer &) override;
			void broadcast(bool force) override;

		protected:
			void broadcast(const TileEntityPacket &);
	};
}
