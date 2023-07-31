#include "entity/Player.h"
#include "game/Inventory.h"
#include "game/ServerInventory.h"
#include "packet/OpenAgentInventoryPacket.h"
#include "packet/TileEntityPacket.h"
#include "realm/Realm.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	InventoriedTileEntity::InventoriedTileEntity(InventoryPtr inventory_):
		HasInventory(std::move(inventory_)) {}

	bool InventoriedTileEntity::canInsertItem(const ItemStack &stack, Direction direction) {
		if (!inventory || !mayInsertItem(stack, direction))
			return false;
		return inventory->canInsert(stack);
	}

	std::optional<ItemStack> InventoriedTileEntity::extractItem(Direction, bool remove) {
		if (!inventory)
			return std::nullopt;

		if (remove) {
			std::optional<ItemStack> out;
			Slot slot = -1;
			ItemStack *stack = inventory->firstItem(&slot);
			if (slot == Slot(-1) || stack == nullptr)
				return std::nullopt;
			out = std::move(*stack);
			inventory->erase(slot);
			inventory->notifyOwner();
			return out;
		}

		ItemStack *stack = inventory->firstItem(nullptr);
		if (stack == nullptr)
			return std::nullopt;
		return *stack;
	}

	bool InventoriedTileEntity::insertItem(const ItemStack &stack, Direction direction, std::optional<ItemStack> *leftover) {
		if (!canInsertItem(stack, direction))
			return false;

		if (leftover != nullptr)
			*leftover = inventory->add(stack);
		else
			inventory->add(stack);

		return true;
	}

	ItemCount InventoriedTileEntity::itemsInsertable(const ItemStack &stack, Direction direction, Slot slot) {
		if (!mayInsertItem(stack, direction))
			return 0;

		return inventory->insertable(stack, slot);
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

	void InventoriedTileEntity::addObserver(const std::shared_ptr<Player> &player) {
		Observable::addObserver(player);
		player->send(TileEntityPacket(shared_from_this()));
		player->send(OpenAgentInventoryPacket(getGID()));
		player->queueForMove([this](const std::shared_ptr<Entity> &entity) {
			removeObserver(std::static_pointer_cast<Player>(entity));
			return true;
		});
	}

	void InventoriedTileEntity::absorbJSON(Game &, const nlohmann::json &) {}

	void InventoriedTileEntity::encode(Game &, Buffer &buffer) {
		HasInventory::encode(buffer);
	}

	void InventoriedTileEntity::decode(Game &, Buffer &buffer) {
		HasInventory::decode(buffer);
	}

	void InventoriedTileEntity::broadcast() {
		if (forceBroadcast)
			TileEntity::broadcast();
		else
			broadcast(TileEntityPacket(shared_from_this()));
	}

	void InventoriedTileEntity::broadcast(const TileEntityPacket &packet) {
		assert(getSide() == Side::Server);
		auto lock = observers.uniqueLock();

		std::erase_if(observers, [&](const std::weak_ptr<Player> &weak_player) {
			if (auto player = weak_player.lock()) {
				player->send(packet);
				return false;
			}

			return true;
		});
	}
}
