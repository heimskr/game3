#pragma once

#include "game/StorageInventory.h"

namespace Game3 {
	class Agent;
	class Buffer;
	struct CraftingRecipe;

	class ClientInventory: public StorageInventory {
		public:
			ClientInventory() = default;
			ClientInventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot = 0, Storage = {});
			ClientInventory(const ClientInventory &);
			ClientInventory(ClientInventory &&);

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

			inline Glib::RefPtr<Gdk::Pixbuf> getImage(const Game &game, Slot slot) { return storage.at(slot).getImage(game); }

		protected:

			static ClientInventory fromJSON(Game &, const nlohmann::json &, const std::shared_ptr<Agent> &);

			friend void to_json(nlohmann::json &, const Inventory &);
	};

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	ClientInventory popBuffer<ClientInventory>(Buffer &);
	Buffer & operator+=(Buffer &, const ClientInventory &);
	Buffer & operator<<(Buffer &, const ClientInventory &);
	Buffer & operator>>(Buffer &, ClientInventory &);

	void to_json(nlohmann::json &, const ClientInventory &);
}
