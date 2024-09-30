#pragma once

#include "game/StorageInventory.h"

namespace Game3 {
	class Agent;
	class Buffer;
	struct CraftingRecipe;

	class ServerInventory: public StorageInventory {
		public:
			ServerInventory() = default;
			ServerInventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot = 0, InventoryID = 0, Storage = {});

			std::unique_ptr<Inventory> copy() const override;

			ItemStackPtr add(const ItemStackPtr &, const SlotPredicate &, Slot start) override;

			void drop(Slot) override;

			void discard(Slot) override;

			void swap(Slot, Slot) override;

			void erase(Slot) override;

			void clear() override;

			ItemCount remove(const ItemStackPtr &) override;
			ItemCount remove(const ItemStackPtr &, const Predicate &) override;
			ItemCount remove(const ItemStackPtr &, Slot) override;
			ItemCount remove(const CraftingRequirement &, const Predicate &) override;
			ItemCount remove(const AttributeRequirement &, const Predicate &) override;

			void setActive(Slot, bool force) override;

			void notifyOwner(std::optional<std::variant<ItemStackPtr, Slot>>) override;

			static ServerInventory fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &, const std::shared_ptr<Agent> &);

			friend void to_json(nlohmann::json &, const ServerInventory &);
	};

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	ServerInventory popBuffer<ServerInventory>(Buffer &);
	Buffer & operator+=(Buffer &, const ServerInventory &);
	Buffer & operator<<(Buffer &, const ServerInventory &);
	Buffer & operator>>(Buffer &, ServerInventory &);

	void to_json(nlohmann::json &, const ServerInventory &);
}
