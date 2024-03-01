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
#include "packet/DropItemPacket.h"
#include "packet/InventoryPacket.h"
#include "packet/SetActiveSlotPacket.h"
#include "packet/SwapSlotsPacket.h"
#include "realm/Realm.h"
#include "recipe/CraftingRecipe.h"
#include "ui/MainWindow.h"
#include "util/Util.h"

namespace Game3 {
	ClientInventory::ClientInventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot, InventoryID index_, Storage storage_):
		StorageInventory(std::move(owner), slot_count, active_slot, index_, std::move(storage_)) {}

	ItemStackPtr ClientInventory::add(const ItemStackPtr &, const SlotPredicate &, Slot) {
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
			send(SwapSlotsPacket(gid, gid, source, destination, index, index));
		}
	}

	void ClientInventory::erase(Slot) {
		throw std::logic_error("ClientInventory::erase(Slot) unimplemented");
	}

	void ClientInventory::clear() {
		storage.clear();
	}

	ItemCount ClientInventory::remove(const ItemStackPtr &) {
		throw std::logic_error("ClientInventory::remove(const ItemStackPtr &) unimplemented");
	}

	ItemCount ClientInventory::remove(const ItemStackPtr &, const Predicate &) {
		throw std::logic_error("ClientInventory::remove(const ItemStackPtr &, const Predicate &) unimplemented");
	}

	ItemCount ClientInventory::remove(const ItemStackPtr &, Slot) {
		throw std::logic_error("ClientInventory::remove(const ItemStackPtr &, Slot) unimplemented");
	}

	ItemCount ClientInventory::remove(const CraftingRequirement &, const Predicate &) {
		throw std::logic_error("ClientInventory::remove(const CraftingRequirement &, const Predicate &) unimplemented");
	}

	ItemCount ClientInventory::remove(const AttributeRequirement &, const Predicate &) {
		throw std::logic_error("ClientInventory::remove(const AttributeRequirement &, const Predicate &) unimplemented");
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
		if (AgentPtr owner = weakOwner.lock()) {
			owner->inventoryUpdated(index);

			ClientGame &game = owner->getRealm()->getGame()->toClient();

			game.getWindow().queue([&game, weak = weakOwner, index = index.load()] {
				if (auto owner = weak.lock()) {
					if (auto player = std::dynamic_pointer_cast<Player>(owner))
						game.signalPlayerInventoryUpdate().emit(player);
					else
						game.signalOtherInventoryUpdate().emit(owner, index);
					return;
				}

				ERROR("Expired in {}:{}", __FILE__, __LINE__);
			});
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
		buffer += inventory.getSlotCount();
		buffer += inventory.activeSlot.load();
		buffer += inventory.index.load();
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
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected inventory)");
		}
		const auto gid = buffer.take<GlobalID>();
		if (auto locked = inventory.weakOwner.lock())
			locked->setGID(gid);
		inventory.setSlotCount(buffer.take<Slot>());
		inventory.activeSlot = buffer.take<Slot>();
		inventory.index = buffer.take<InventoryID>();
		inventory.setStorage(buffer.take<std::decay_t<decltype(inventory.getStorage())>>());
		return buffer;
	}

	void to_json(nlohmann::json &json, const ClientInventory &inventory) {
		for (const auto &[key, val]: inventory.getStorage())
			json["storage"][std::to_string(key)] = *val;
		json["slotCount"]  = inventory.getSlotCount();
		json["activeSlot"] = inventory.activeSlot.load();
	}
}
