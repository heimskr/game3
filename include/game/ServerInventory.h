#pragma once

#include "game/StorageInventory.h"

namespace Game3 {
	class Agent;
	class Buffer;
	struct CraftingRecipe;

	class ServerInventory: public StorageInventory {
		public:
			ServerInventory() = default;
			ServerInventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot = 0, Storage = {});

			std::optional<ItemStack> add(const ItemStack &, Slot start) override;

			void drop(Slot) override;

			void discard(Slot) override;

			bool swap(Slot, Slot) override;

			void erase(Slot, bool suppress_notification) override;

			ItemCount remove(const ItemStack &) override;
			ItemCount remove(const ItemStack &, Slot) override;
			ItemCount remove(const CraftingRequirement &) override;
			ItemCount remove(const AttributeRequirement &) override;

			void setActive(Slot, bool force) override;

			void notifyOwner() override;

		protected:
			static ServerInventory fromJSON(Game &, const nlohmann::json &, const std::shared_ptr<Agent> &);

			friend void to_json(nlohmann::json &, const Inventory &);
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
