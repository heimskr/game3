#pragma once

#include "types/Types.h"

#include <memory>

namespace Game3 {
	class Agent;
	class Buffer;
	class Game;
	class Inventory;

	class HasInventory {
		public:
			HasInventory(std::shared_ptr<Inventory> inventory_ = nullptr):
				inventory(std::move(inventory_)) {}

			virtual ~HasInventory() = default;

			virtual const std::shared_ptr<Inventory> & getInventory(InventoryID) const;
			virtual void setInventory(std::shared_ptr<Inventory>, InventoryID);
			virtual InventoryID getInventoryCount() const { return 1; }

			void encode(Buffer &, InventoryID);
			void decode(Buffer &, InventoryID);

			virtual void inventoryUpdated(InventoryID) {}
			virtual std::shared_ptr<Agent> getSharedAgent() = 0;

			template <typename T>
			void decodeSpecific(Buffer &buffer, InventoryID index) {
				Slot slot_count = -1;
				buffer >> slot_count;
				std::shared_ptr<T> inventory;
				if (slot_count == -1) {
					setInventory(nullptr, index);
					inventoryUpdated(index);
					buffer >> inventory;
					assert(!inventory);
				} else {
					buffer >> inventory;
					assert(inventory);
					if (inventory) { // This is unnecessary but I want PVS-Studio to be happy.
						inventory->weakOwner = getSharedAgent();
						inventory->setSlotCount(slot_count); // Maybe not necessary? Try an assert before.
						inventory->index = index;
						setInventory(inventory, index);
						inventoryUpdated(index);
					}
				}
			}

		private:
			std::shared_ptr<Inventory> inventory;
	};
}
