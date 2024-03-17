#pragma once

#include "game/Inventory.h"


namespace Game3 {
	// Disgraceful.

	class InventoryWrapper: public Inventory {
		protected:
			std::shared_ptr<Inventory> inventory;
			virtual bool validateSlot(Slot) const { return true; }
			virtual Slot adjustSlot(Slot slot) const { return slot; }

		public:
			InventoryWrapper(std::shared_ptr<Inventory>);

			Slot getSlotCount() const override;
			void setSlotCount(Slot) override;
			void set(Slot, ItemStackPtr) override;
			std::unique_ptr<Inventory> copy() const override;
			ItemStackPtr operator[](Slot) const override;
			void iterate(const Predicate &) const override;
			ItemStackPtr firstItem(Slot *slot_out) override;
			ItemStackPtr firstItem(Slot *slot_out, const Predicate &) override;
			ItemStackPtr add(const ItemStackPtr &, const SlotPredicate &predicate, Slot start) override;
			bool canInsert(const ItemStackPtr &, const SlotPredicate &) const override;
			bool canInsert(const ItemStackPtr &, Slot) const override;
			bool canInsert(const ItemStackPtr &) const override;
			bool canExtract(Slot) const override;
			ItemCount insertable(const ItemStackPtr &, Slot) const override;
			void drop(Slot) override;
			void discard(Slot) override;
			void swap(Slot, Slot) override;
			void erase(Slot) override;
			void clear() override;
			ItemCount count(const ItemID &) const override;
			ItemCount count(const Item &) const override;
			ItemCount count(const ItemStackPtr &) const override;
			ItemCount count(const ItemStackPtr &, const SlotPredicate &) const override;
			ItemCount countAttribute(const Identifier &) const override;
			bool hasSlot(Slot) const override;
			ItemStackPtr front() const override;
			ItemCount remove(const ItemStackPtr &) override;
			ItemCount remove(const ItemStackPtr &, const Predicate &) override;
			ItemCount remove(const ItemStackPtr &, Slot) override;
			ItemCount remove(const CraftingRequirement &, const Predicate &) override;
			ItemCount remove(const AttributeRequirement &, const Predicate &) override;
			bool contains(Slot) const override;
			bool contains(const ItemStackPtr &) const override;
			bool contains(const ItemStackPtr &, const Predicate &) const override;
			std::optional<Slot> find(const ItemID &) const override;
			std::optional<Slot> find(const ItemID &, const Predicate &) const override;
			std::optional<Slot> findAttribute(const Identifier &) const override;
			std::optional<Slot> findAttribute(const Identifier &, const Predicate &) const override;
			ItemStackPtr getActive() const override;
			void setActive(Slot, bool force) override;
			void notifyOwner() override;
			bool empty() const override;
			Slot slotsOccupied() const override;
			void replace(const Inventory &) override;
			void replace(Inventory &&) override;

			using Inventory::add;

			std::unique_lock<DefaultMutex> uniqueLock() const override;
			std::shared_lock<DefaultMutex> sharedLock() const override;

		protected:
			void compact() override;
	};
}
