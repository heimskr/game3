#include "Log.h"
#include "entity/Player.h"
#include "game/Inventory.h"
#include "game/ServerInventory.h"
#include "packet/OpenModuleForAgentPacket.h"
#include "packet/TileEntityPacket.h"
#include "realm/Realm.h"
#include "tileentity/InventoriedTileEntity.h"
#include "ui/module/InventoryModule.h"
#include "util/Cast.h"

namespace Game3 {
	InventoriedTileEntity::InventoriedTileEntity(InventoryPtr inventory_):
		HasInventory(std::move(inventory_)) {}

	bool InventoriedTileEntity::canInsertItem(const ItemStackPtr &stack, Direction direction, Slot slot) {
		// TODO: support multiple inventories with canInsertItem
		const InventoryPtr inventory = getInventory(0);
		if (!inventory || !mayInsertItem(stack, direction, slot))
			return false;
		return inventory->canInsert(stack);
	}

	bool InventoriedTileEntity::canExtractItem(Direction direction, Slot slot) {
		// TODO: support multiple inventories with canExtractItem
		const InventoryPtr inventory = getInventory(0);
		if (!inventory || !mayExtractItem(direction, slot))
			return false;
		return inventory->canExtract(slot);
	}

	std::optional<ItemStack> InventoriedTileEntity::extractItem(Direction, bool remove, Slot slot) {
		// TODO: support multiple inventories with extractItem
		const InventoryPtr inventory = getInventory(0);

		if (!inventory)
			return std::nullopt;

		ItemStack *stack = nullptr;

		auto inventory_lock = inventory->uniqueLock();

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

	bool InventoriedTileEntity::InsertItem(const ItemStackPtr &stack, Direction direction, std::optional<ItemStack> *leftover) {
		if (!mayInsertItem(stack, direction))
			return false;

		auto predicate = [&](Slot slot) {
			return mayInsertItem(stack, direction, slot);
		};

		// TODO: support multiple inventories with insertItem
		const InventoryPtr inventory = getInventory(0);
		assert(inventory);
		auto inventory_lock = inventory->uniqueLock();

		if (leftover != nullptr)
			*leftover = inventory->add(stack, predicate);
		else
			inventory->add(stack, predicate);

		inventory->notifyOwner();
		return true;
	}

	ItemCount InventoriedTileEntity::itemsInsertable(const ItemStack &stack, Direction direction, Slot slot) {
		if (!mayInsertItem(stack, direction))
			return 0;

		// TODO: support multiple inventories with itemsInsertable
		return getInventory(0)->insertable(stack, slot);
	}

	void InventoriedTileEntity::iterateExtractableItems(Direction direction, const std::function<bool(const ItemStack &, Slot)> &function) {
		// TODO: support multiple inventories with iterateExtractableItems
		getInventory(0)->iterate([&](const ItemStack &stack, Slot slot) {
			return canExtractItem(direction, slot) && function(stack, slot);
		});
	}

	bool InventoriedTileEntity::empty() const {
		// TODO: support multiple inventories with empty
		const InventoryPtr inventory = getInventory(0);
		return !inventory || inventory->empty();
	}

	void InventoriedTileEntity::setInventory(Slot slot_count) {
		RealmPtr realm = weakRealm.lock();
		assert(realm);
		// TODO: support multiple inventories with setInventory
		HasInventory::setInventory(Inventory::create(realm->getSide(), shared_from_this(), slot_count), 0);
		inventoryUpdated();
	}

	void InventoriedTileEntity::inventoryUpdated() {
		if (getSide() != Side::Server)
			return;
		increaseUpdateCounter();
	}

	std::shared_ptr<Agent> InventoriedTileEntity::getSharedAgent() {
		return shared_from_this();
	}

	void InventoriedTileEntity::addObserver(const PlayerPtr &player, bool silent) {
		Observable::addObserver(player, silent);

		player->send(TileEntityPacket(getSelf()));

		if (!silent)
			player->send(OpenModuleForAgentPacket(InventoryModule::ID(), getGID()));

		player->queueForMove([weak_self = getWeakSelf()](const EntityPtr &entity, bool) {
			if (auto self = weak_self.lock())
				safeDynamicCast<InventoriedTileEntity>(self)->removeObserver(safeDynamicCast<Player>(entity));
			return true;
		});
	}

	void InventoriedTileEntity::absorbJSON(const std::shared_ptr<Game> &, const nlohmann::json &) {}

	void InventoriedTileEntity::encode(Game &, Buffer &buffer) {
		HasInventory::encode(buffer, 0);
	}

	void InventoriedTileEntity::decode(Game &, Buffer &buffer) {
		HasInventory::decode(buffer, 0);
	}

	void InventoriedTileEntity::broadcast(bool force) {
		if (force)
			TileEntity::broadcast(true);
		else
			broadcast(TileEntityPacket(getSelf()));
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
