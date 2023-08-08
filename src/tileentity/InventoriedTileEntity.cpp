#include "entity/Player.h"
#include "game/Inventory.h"
#include "game/ServerInventory.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "packet/TileEntityPacket.h"
#include "realm/Realm.h"
#include "tileentity/InventoriedTileEntity.h"
#include "ui/module/ExternalInventoryModule.h"

namespace Game3 {
	InventoriedTileEntity::InventoriedTileEntity(InventoryPtr inventory_):
		HasInventory(std::move(inventory_)) {}

	bool InventoriedTileEntity::canInsertItem(const ItemStack &stack, Direction direction, Slot slot) {
		if (!inventory || !mayInsertItem(stack, direction, slot))
			return false;
		return inventory->canInsert(stack);
	}

	bool InventoriedTileEntity::canExtractItem(Direction direction, Slot slot) {
		if (!inventory || !mayExtractItem(direction, slot))
			return false;
		return inventory->canExtract(slot);
	}

	std::optional<ItemStack> InventoriedTileEntity::extractItem(Direction, bool remove, Slot slot) {
		if (!inventory)
			return std::nullopt;

		ItemStack *stack = nullptr;

		if (remove) {
			std::optional<ItemStack> out;

			if (slot == Slot(-1)) {
				Slot slot_extracted_from = -1;
				stack = inventory->firstItem(&slot_extracted_from);
				if (slot_extracted_from == Slot(-1) || stack == nullptr)
					return std::nullopt;
				out = std::move(*stack);
				inventory->erase(slot_extracted_from);
			} else {
				stack = (*inventory)[slot];
				if (stack == nullptr)
					return std::nullopt;
				out = std::move(*stack);
				inventory->erase(slot);
			}

			inventory->notifyOwner();
			return out;
		}

		if (slot == Slot(-1))
			stack = inventory->firstItem(nullptr);
		else
			stack = (*inventory)[slot];

		if (stack == nullptr)
			return std::nullopt;

		return *stack;
	}

	bool InventoriedTileEntity::insertItem(const ItemStack &stack, Direction direction, std::optional<ItemStack> *leftover) {
		if (!mayInsertItem(stack, direction))
			return false;

		auto predicate = [&](Slot slot) {
			return mayInsertItem(stack, direction, slot);
		};

		if (leftover != nullptr)
			*leftover = inventory->add(stack, predicate);
		else
			inventory->add(stack, predicate);

		return true;
	}

	ItemCount InventoriedTileEntity::itemsInsertable(const ItemStack &stack, Direction direction, Slot slot) {
		if (!mayInsertItem(stack, direction))
			return 0;

		return inventory->insertable(stack, slot);
	}

	void InventoriedTileEntity::iterateExtractableItems(Direction direction, const std::function<bool(const ItemStack &, Slot)> &function) {
		inventory->iterate([&](const ItemStack &stack, Slot slot) {
			return canExtractItem(direction, slot) && function(stack, slot);
		});
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
		player->send(OpenModuleForAgentPacket(ExternalInventoryModule::ID(), getGID()));
		player->queueForMove([this, self = shared_from_this()](const std::shared_ptr<Entity> &entity) {
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
