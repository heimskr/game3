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
#include "packet/ActiveSlotSetPacket.h"
#include "packet/DropItemPacket.h"
#include "packet/InventoryPacket.h"
#include "packet/SetActiveSlotPacket.h"
#include "packet/SwapSlotsPacket.h"
#include "realm/Realm.h"
#include "recipe/CraftingRecipe.h"
#include "util/Util.h"

namespace Game3 {
	ClientInventory::ClientInventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot, Storage storage_):
		StorageInventory(std::move(owner), slot_count, active_slot, std::move(storage_)) {}

	std::optional<ItemStack> ClientInventory::add(const ItemStack &, const std::function<bool(Slot)> &, Slot) {
		throw std::logic_error("Clients cannot add items to inventories");
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

	std::unique_ptr<Inventory> ClientInventory::copy() const {
		return std::make_unique<ClientInventory>(*this);
	}

	void ClientInventory::drop(Slot slot) {
		send(DropItemPacket(slot, false));
	}

	void ClientInventory::discard(Slot slot) {
		send(DropItemPacket(slot, true));
	}

	void ClientInventory::swap(Slot source, Slot destination) {
		if (auto owner = weakOwner.lock()) {
			GlobalID gid = owner->getGID();
			send(SwapSlotsPacket(gid, gid, source, destination));
		}
	}

	void ClientInventory::erase(Slot) {
		throw std::logic_error("ClientInventory::erase(Slot) unimplemented");
	}

	ItemCount ClientInventory::remove(const ItemStack &) {
		throw std::logic_error("ClientInventory::remove(const ItemStack &) unimplemented");
	}

	ItemCount ClientInventory::remove(const ItemStack &, const std::function<bool(Slot)> &) {
		throw std::logic_error("ClientInventory::remove(const ItemStack &, const std::function<bool(Slot)> &) unimplemented");
	}

	ItemCount ClientInventory::remove(const ItemStack &, Slot) {
		throw std::logic_error("ClientInventory::remove(const ItemStack &, Slot) unimplemented");
	}

	ItemCount ClientInventory::remove(const CraftingRequirement &) {
		throw std::logic_error("ClientInventory::remove(const CraftingRequirement &) unimplemented");
	}

	ItemCount ClientInventory::remove(const AttributeRequirement &) {
		throw std::logic_error("ClientInventory::remove(const AttributeRequirement &) unimplemented");
	}

	void ClientInventory::setActive(Slot new_active, bool force) {
		if (!std::dynamic_pointer_cast<Player>(weakOwner.lock()))
			throw std::runtime_error("Can't set the active slot of a non-player inventory");

		if (force)
			activeSlot = new_active;
		else
			send(SetActiveSlotPacket(new_active));
	}

	void ClientInventory::notifyOwner() {
		if (auto owner = weakOwner.lock()) {
			if (auto player = std::dynamic_pointer_cast<Player>(owner))
				player->getRealm()->getGame().toClient().signal_player_inventory_update().emit(player);
			else
				owner->getRealm()->getGame().toClient().signal_other_inventory_update().emit(owner);
		}
	}

	void ClientInventory::send(const Packet &packet) {
		auto owner = weakOwner.lock();
		if (!owner)
			throw std::logic_error("ClientInventory is missing an owner");

		if (auto player = std::dynamic_pointer_cast<Player>(owner))
			player->send(packet);
		else
			throw std::runtime_error("Can't send packets from a non-player-owned ClientInventory");
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
			auto lock = storage.sharedLock();
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
