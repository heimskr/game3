#include "game/Inventory.h"
#include "game/ServerInventory.h"
#include "realm/Realm.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	InventoriedTileEntity::InventoriedTileEntity(InventoryPtr inventory_):
		HasInventory(std::move(inventory_)) {}

	bool InventoriedTileEntity::canInsertItem(const ItemStack &stack, Direction) {
		if (!inventory)
			return false;

		auto lock = inventory->sharedLock();
		return inventory->canInsert(stack);
	}

	std::optional<ItemStack> InventoriedTileEntity::extractItem(Direction, bool remove) {
		if (!inventory)
			return std::nullopt;

		if (remove) {
			std::optional<ItemStack> out;
			{
				auto lock = inventory->uniqueLock();
				Slot slot = -1;
				ItemStack *stack = inventory->firstItem(&slot);
				if (slot == Slot(-1) || stack == nullptr)
					return std::nullopt;
				out = std::move(*stack);
				inventory->erase(slot);
			}
			inventory->notifyOwner();
			inventoryUpdated();
			return out;
		}

		auto lock = inventory->sharedLock();
		ItemStack *stack = inventory->firstItem(nullptr);
		if (stack == nullptr)
			return std::nullopt;
		return *stack;
	}

	bool InventoriedTileEntity::empty() const {
		return !inventory || inventory->empty();
	}

	void InventoriedTileEntity::setInventory(Slot slot_count) {
		if (auto realm = weakRealm.lock())
			assert(realm->getSide() == Side::Server);
		inventory = std::make_shared<ServerInventory>(shared_from_this(), slot_count);
		inventoryUpdated();
	}

	void InventoriedTileEntity::inventoryUpdated() {
		increaseUpdateCounter();
	}

	std::shared_ptr<Agent> InventoriedTileEntity::getSharedAgent() {
		return shared_from_this();
	}

	void InventoriedTileEntity::absorbJSON(Game &, const nlohmann::json &) {}

	void InventoriedTileEntity::encode(Game &, Buffer &buffer) {
		HasInventory::encode(buffer);
	}

	void InventoriedTileEntity::decode(Game &, Buffer &buffer) {
		HasInventory::decode(buffer);
	}
}
