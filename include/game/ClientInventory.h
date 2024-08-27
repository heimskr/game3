#pragma once

#include "game/StorageInventory.h"

namespace Game3 {
	class Agent;
	class Buffer;
	class Packet;
	struct CraftingRecipe;

	class ClientInventory: public StorageInventory {
		public:
			ClientInventory() = default;
			ClientInventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot = 0, InventoryID = 0, Storage = {});
			ClientInventory(const ClientInventory &);
			ClientInventory(ClientInventory &&) noexcept;

			ClientInventory & operator=(const ClientInventory &);
			ClientInventory & operator=(ClientInventory &&) noexcept;

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

			void notifyOwner() override;

			inline Glib::RefPtr<Gdk::Pixbuf> getImage(const Game &game, Slot slot) { return storage.at(slot)->getImage(game); }

		private:
			void send(const Packet &);

			static ClientInventory fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &, const std::shared_ptr<Agent> &);

			friend void to_json(nlohmann::json &, const Inventory &);
	};

	using ClientInventoryPtr = std::shared_ptr<ClientInventory>;

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	ClientInventory popBuffer<ClientInventory>(Buffer &);
	Buffer & operator+=(Buffer &, const ClientInventory &);
	Buffer & operator<<(Buffer &, const ClientInventory &);
	Buffer & operator>>(Buffer &, ClientInventory &);

	void to_json(nlohmann::json &, const ClientInventory &);
}
