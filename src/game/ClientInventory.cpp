#include <cassert>

#include "Log.h"
#include "entity/Entity.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "entity/ServerPlayer.h"
#include "game/Agent.h"
#include "game/ClientGame.h"
#include "game/ClientInventory.h"
#include "net/Buffer.h"
#include "packet/InventoryPacket.h"
#include "packet/SetActiveSlotPacket.h"
#include "packet/ActiveSlotSetPacket.h"
#include "packet/DropItemPacket.h"
#include "realm/Realm.h"
#include "recipe/CraftingRecipe.h"
#include "util/Util.h"

namespace Game3 {
	ClientInventory::ClientInventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot, Storage storage_):
		StorageInventory(std::move(owner), slot_count, active_slot, std::move(storage_)) {}

	std::optional<ItemStack> ClientInventory::add(const ItemStack &stack, Slot start) {
		throw std::runtime_error("Clients cannot add items to inventories");
	}

	ClientInventory::ClientInventory(const ClientInventory &other):
		StorageInventory(other) {}

	ClientInventory::ClientInventory(ClientInventory &&other):
		StorageInventory(std::move(other)) {}


	ClientInventory & ClientInventory::operator=(const ClientInventory &other) {
		StorageInventory::operator=(other);
		return *this;
	}

	ClientInventory & ClientInventory::operator=(ClientInventory &&other) {
		StorageInventory::operator=(std::move(other));
		return *this;
	}

	void ClientInventory::drop(Slot slot) {
		auto owner = weakOwner.lock();
		if (!owner)
			throw std::logic_error("ClientInventory is missing an owner");

		if (auto player = std::dynamic_pointer_cast<Player>(owner))
			player->send(DropItemPacket(slot, false));
		else
			throw std::runtime_error("Only Players can drop items on the client");
	}

	void ClientInventory::discard(Slot slot) {
		auto owner = weakOwner.lock();
		if (!owner)
			throw std::logic_error("ClientInventory is missing an owner");

		if (auto player = std::dynamic_pointer_cast<Player>(owner))
			player->send(DropItemPacket(slot, true));
		else
			throw std::runtime_error("Only Players can discard items on the client");
	}

	bool ClientInventory::swap(Slot source, Slot destination) {
		// TODO
		return true;
	}

	void ClientInventory::erase(Slot slot, bool suppress_notification) {
		// TODO
		if (!suppress_notification)
			notifyOwner();
	}

	ItemCount ClientInventory::remove(const ItemStack &stack_to_remove) {
		// TODO
		return 0;
	}

	ItemCount ClientInventory::remove(const ItemStack &stack_to_remove, Slot slot) {
		// TODO
		return 0;
	}

	ItemCount ClientInventory::remove(const CraftingRequirement &requirement) {
		if (requirement.is<ItemStack>())
			return remove(requirement.get<ItemStack>());
		return remove(requirement.get<AttributeRequirement>());
	}

	ItemCount ClientInventory::remove(const AttributeRequirement &requirement) {
		// TODO
		return 0;
	}

	void ClientInventory::setActive(Slot new_active, bool force) {
		// TODO
	}

	void ClientInventory::notifyOwner() {
		if (auto owner = weakOwner.lock()) {
			if (auto player = std::dynamic_pointer_cast<Player>(owner))
				player->getRealm()->getGame().toClient().signal_player_inventory_update().emit(player);
			else
				owner->getRealm()->getGame().toClient().signal_other_inventory_update().emit(owner);
		}
	}

	template <>
	std::string Buffer::getType(const ClientInventory &) {
		return "\xe1";
	}

	Buffer & operator+=(Buffer &buffer, const ClientInventory &inventory) {
		buffer.appendType(inventory);
		if (auto locked = inventory.weakOwner.lock())
			buffer += locked->getGID();
		else
			buffer += static_cast<GlobalID>(-1);
		buffer += inventory.slotCount.load();
		buffer += inventory.activeSlot.load();
		{
			auto &storage = inventory.getStorage();
			auto lock = const_cast<Lockable<ClientInventory::Storage> &>(storage).sharedLock();
			buffer += storage.getBase();
		}
		return buffer;
	}

	Buffer & operator<<(Buffer &buffer, const ClientInventory &inventory) {
		return buffer += inventory;
	}

	Buffer & operator>>(Buffer &buffer, ClientInventory &inventory) {
		const auto type = buffer.popType();
		if (!buffer.typesMatch(type, buffer.getType(inventory))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type) + ") in buffer (expected inventory)");
		}
		const auto gid = popBuffer<GlobalID>(buffer);
		if (auto locked = inventory.weakOwner.lock())
			locked->setGID(gid);
		inventory.slotCount = popBuffer<Slot>(buffer);
		inventory.activeSlot = popBuffer<Slot>(buffer);
		inventory.setStorage(popBuffer<std::decay_t<decltype(inventory.getStorage())>>(buffer));
		return buffer;
	}
}
