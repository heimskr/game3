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

			friend ServerInventory tag_invoke(boost::json::value_to_tag<ServerInventory>, const boost::json::value &, const std::pair<GamePtr, AgentPtr> &);
			friend void tag_invoke(boost::json::value_from_tag, boost::json::value &, const ServerInventory &);
	};

	ServerInventory tag_invoke(boost::json::value_to_tag<ServerInventory>, const boost::json::value &, const std::pair<GamePtr, AgentPtr> &);

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	ServerInventory popBuffer<ServerInventory>(Buffer &);
	Buffer & operator+=(Buffer &, const ServerInventory &);
	Buffer & operator<<(Buffer &, const ServerInventory &);
	Buffer & operator>>(Buffer &, ServerInventory &);

	ServerInventory tag_invoke(boost::json::value_to_tag<ServerInventory>, const boost::json::value &, const std::pair<GamePtr, AgentPtr> &);
	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const ServerInventory &);
}
