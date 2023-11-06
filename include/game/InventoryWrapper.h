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

			std::unique_ptr<Inventory> copy() const override;
			ItemStack * operator[](Slot) override;
			const ItemStack * operator[](Slot) const override;
			void iterate(const ConstPredicate &) const override;
			void iterate(const Predicate &) override;
			ItemStack * firstItem(Slot *slot_out) override;
			ItemStack * firstItem(Slot *slot_out, const ConstPredicate &) override;
			std::optional<ItemStack> add(const ItemStack &, const SlotPredicate &predicate, Slot start) override;
			bool canInsert(const ItemStack &, const SlotPredicate &) const override;
			bool canInsert(const ItemStack &, Slot) const override;
			bool canInsert(const ItemStack &) const override;
			bool canExtract(Slot) const override;
			ItemCount insertable(const ItemStack &, Slot) const override;
			void drop(Slot) override;
			void discard(Slot) override;
			void swap(Slot, Slot) override;
			void erase(Slot) override;
			void clear() override;
			ItemCount count(const ItemID &) const override;
			ItemCount count(const Item &) const override;
			ItemCount count(const ItemStack &) const override;
			ItemCount count(const ItemStack &, const SlotPredicate &) const override;
			ItemCount countAttribute(const Identifier &) const override;
			bool hasSlot(Slot) const override;
			ItemStack & front() override;
			const ItemStack & front() const override;
			ItemCount remove(const ItemStack &) override;
			ItemCount remove(const ItemStack &, const ConstPredicate &) override;
			ItemCount remove(const ItemStack &, Slot) override;
			ItemCount remove(const CraftingRequirement &, const ConstPredicate &) override;
			ItemCount remove(const AttributeRequirement &, const ConstPredicate &) override;
			bool contains(Slot) const override;
			bool contains(const ItemStack &) const override;
			bool contains(const ItemStack &, const ConstPredicate &) const override;
			std::optional<Slot> find(const ItemID &) const override;
			std::optional<Slot> find(const ItemID &, const ConstPredicate &) const override;
			std::optional<Slot> findAttribute(const Identifier &) const override;
			std::optional<Slot> findAttribute(const Identifier &, const ConstPredicate &) const override;
			ItemStack * getActive() override;
			const ItemStack * getActive() const override;
			void setActive(Slot, bool force) override;
			void notifyOwner() override;
			bool empty() const override;

			using Inventory::add;

			std::unique_lock<DefaultMutex> uniqueLock() const override;
			std::shared_lock<DefaultMutex> sharedLock() const override;

		protected:
			void compact() override;
	};
}
