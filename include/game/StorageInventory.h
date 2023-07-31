#pragma once

#include "game/Inventory.h"

namespace Game3 {
	class StorageInventory: public Inventory {
		public:
			using Storage = std::map<Slot, ItemStack>;

		protected:
			Lockable<Storage> storage;

			StorageInventory() = default;
			StorageInventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot, Storage);
			StorageInventory(const StorageInventory &);
			StorageInventory(StorageInventory &&);

		public:
			StorageInventory & operator=(const StorageInventory &);
			StorageInventory & operator=(StorageInventory &&);

			ItemStack * operator[](size_t) override;
			const ItemStack * operator[](size_t) const override;

			void iterate(const std::function<bool(const ItemStack &, Slot)> &) override;

			ItemStack * firstItem(Slot *slot_out) override;

			bool canInsert(const ItemStack &) const override;
			bool canInsert(const ItemStack &, Slot) const override;

			ItemCount insertable(const ItemStack &, Slot) const override;

			/** Counts the number of an item in the inventory. */
			ItemCount count(const ItemID &) const override;

			/** Counts the number of an item in the inventory. */
			ItemCount count(const Item &) const override;

			/** Counts the number of an item in the inventory. This takes ItemStack data into account but ignores the given ItemStack's count. */
			ItemCount count(const ItemStack &) const override;

			/** Counts the number of items with a given attribute in the inventory. */
			ItemCount countAttribute(const Identifier &) const override;

			bool hasSlot(Slot) const override;

			bool empty() const override;

			ItemStack & front() override;
			const ItemStack & front() const override;

			bool contains(Slot) const override;

			/** Returns whether the inventory contains at least a minimum amount of a given item. */
			bool contains(const ItemStack &) const override;

			/** Returns the slot containing a given item ID if one exists. */
			std::optional<Slot> find(const ItemID &) const override;

			/** Returns the first slot containing an item with the given attribute if one exists. */
			std::optional<Slot> findAttribute(const Identifier &) const override;

			ItemStack * getActive() override;

			const ItemStack * getActive() const override;

			void prevSlot() override;

			void nextSlot() override;

			inline auto & getStorage() { return storage; }
			inline const auto & getStorage() const { return storage; }
			inline void setStorage(Lockable<Storage> &&new_storage) { storage = std::forward<Lockable<Storage>>(new_storage); }
			inline void setStorage(Storage &&new_storage) { storage = std::forward<Storage>(new_storage); }

		protected:
			/** Removes every slot whose item count is zero from the storage map. */
			void compact() override;
	};
}
