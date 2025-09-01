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

			virtual bool mayInsertItem(const ItemStackPtr &, Direction, Slot) { return true; }
			virtual bool mayInsertItem(const ItemStackPtr &stack, Direction direction) { return mayInsertItem(stack, direction, -1); }
			virtual bool mayExtractItem(Direction, Slot) { return true; }
			virtual bool mayExtractItem(Direction direction) { return mayExtractItem(direction, -1); }
			virtual bool canInsertItem(const ItemStackPtr &, Direction, Slot);
			virtual bool canExtractItem(Direction, Slot);
			/** Returns the extracted item. `max` should be -1 for no maximum or -2 to use the extracted stack's typical max.  */
			virtual ItemStackPtr extractItem(Direction, bool remove, Slot slot, ItemCount max);
			virtual ItemStackPtr extractItem(Direction direction, bool remove, Slot slot) { return extractItem(direction, remove, slot, -1); }
			/** Returns whether the item was insertable at all. */
			virtual bool insertItem(ItemStackPtr, Direction, ItemStackPtr *leftover);
			virtual ItemCount itemsInsertable(const ItemStackPtr &, Direction, Slot);
			/** Iterates over each extractable item until they all have been iterated or the iteration function returns true. */
			virtual void iterateExtractableItems(Direction, const std::function<bool(const ItemStackPtr &, Slot)> &);

			virtual bool empty() const;

			using HasInventory::setInventory;
			virtual void setInventory(Slot slot_count);

			void inventoryUpdated(InventoryID) override;
			std::shared_ptr<Agent> getSharedAgent() final;

			void addObserver(const std::shared_ptr<Player> &, bool silent) override;

			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;
			void encode(Game &, Buffer &) override;
			void decode(Game &, BasicBuffer &) override;
			void broadcast(bool force) override;

		protected:
			void broadcast(const std::shared_ptr<TileEntityPacket> &);
	};
}
