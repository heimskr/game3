#include <cassert>

#include "Log.h"
#include "entity/Entity.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "entity/ServerPlayer.h"
#include "game/Agent.h"
#include "game/ClientGame.h"
#include "game/Inventory.h"
#include "net/Buffer.h"
#include "packet/InventoryPacket.h"
#include "packet/SetActiveSlotPacket.h"
#include "packet/ActiveSlotSetPacket.h"
#include "packet/DropItemPacket.h"
#include "realm/Realm.h"
#include "recipe/CraftingRecipe.h"
#include "util/Util.h"

namespace Game3 {
	Inventory::Inventory(const std::shared_ptr<Agent> &owner, Slot slot_count):
		weakOwner(owner), slotCount(slot_count) {}

	Inventory::Inventory(const Inventory &other):
		weakOwner(other.weakOwner), slotCount(other.slotCount.load()), activeSlot(other.activeSlot.load()), storage(other.storage) {}

	Inventory::Inventory(Inventory &&other):
	weakOwner(other.weakOwner), slotCount(other.slotCount.load()), activeSlot(other.activeSlot.load()), storage(std::move(other.storage)) {
		other.weakOwner = {};
		other.slotCount = 0;
		other.activeSlot = 0;
	}

	ItemStack * Inventory::operator[](size_t slot) {
		if (auto iter = storage.find(slot); iter != storage.end())
			return &iter->second;
		return nullptr;
	}

	const ItemStack * Inventory::operator[](size_t slot) const {
		if (auto iter = storage.find(slot); iter != storage.end())
			return &iter->second;
		return nullptr;
	}

	std::optional<ItemStack> Inventory::add(const ItemStack &stack, Slot start) {
		ssize_t remaining = stack.count;

		if (0 <= start) {
			if (slotCount <= start)
				throw std::out_of_range("Can't start at slot " + std::to_string(start) + ": out of range");
			if (storage.contains(start)) {
				auto &stored = storage.at(start);
				if (stored.canMerge(stack)) {
					const ssize_t storable = ssize_t(stored.item->maxCount) - ssize_t(stored.count);
					if (0 < storable) {
						const ItemCount to_store = std::min(ItemCount(remaining), ItemCount(storable));
						stored.count += to_store;
						remaining -= to_store;
					}
				}
			} else {
				const ItemCount to_store = std::min(ItemCount(stack.item->maxCount), ItemCount(remaining));
				assert(storage.try_emplace(start, getOwner()->getRealm()->getGame(), stack.item, to_store).second);
				remaining -= to_store;
			}
		}

		if (0 < remaining) {
			for (auto &[slot, stored]: storage) {
				if (slot == start || !stored.canMerge(stack))
					continue;
				const ssize_t storable = ssize_t(stored.item->maxCount) - ssize_t(stored.count);
				if (0 < storable) {
					const ItemCount to_store = std::min(ItemCount(remaining), ItemCount(storable));
					stored.count += to_store;
					remaining -= to_store;
					if (remaining <= 0)
						break;
				}
			}
		}

		if (0 < remaining) {
			const Game &game = getOwner()->getRealm()->getGame();
			for (Slot slot = 0; slot < slotCount; ++slot) {
				if (storage.contains(slot))
					continue;
				const ItemCount to_store = std::min(ItemCount(remaining), stack.item->maxCount);
				storage.emplace(slot, ItemStack(game, stack.item, to_store, stack.data));
				remaining -= to_store;
				if (remaining <= 0)
					break;
			}
		}

		notifyOwner();

		if (remaining < 0)
			throw std::logic_error("How'd we end up with " + std::to_string(remaining) + " items remaining?");

		if (remaining == 0)
			return std::nullopt;

		return ItemStack(getOwner()->getRealm()->getGame(), stack.item, remaining);
	}

	bool Inventory::canStore(const ItemStack &stack) const {
		ssize_t remaining = stack.count;

		for (const auto &[slot, stored]: storage) {
			if (!stored.canMerge(stack))
				continue;
			const ssize_t storable = ssize_t(stored.item->maxCount) - ssize_t(stored.count);
			if (0 < storable) {
				const ItemCount to_store = std::min(ItemCount(remaining), ItemCount(storable));
				remaining -= to_store;
				if (remaining <= 0)
					break;
			}
		}

		if (0 < remaining)
			for (Slot slot = 0; slot < slotCount; ++slot) {
				if (storage.contains(slot))
					continue;
				remaining -= std::min(ItemCount(remaining), stack.item->maxCount);
				if (remaining <= 0)
					break;
			}

		if (remaining < 0)
			throw std::logic_error("How'd we end up with " + std::to_string(remaining) + " items remaining?");

		return remaining == 0;
	}

	void Inventory::drop(Slot slot) {
		if (!storage.contains(slot))
			return;

		auto owner = weakOwner.lock();
		if (!owner)
			throw std::logic_error("Inventory is missing an owner");

		if (owner->getSide() == Side::Server) {
			auto realm = owner->getRealm();
			realm->spawn<ItemEntity>(owner->getPosition(), storage.at(slot));
			storage.erase(slot);
			notifyOwner();
		} else if (auto player = std::dynamic_pointer_cast<Player>(owner)) {
			player->send(DropItemPacket(slot, false));
		} else
			throw std::runtime_error("Only Players can drop items on the client");
	}

	void Inventory::discard(Slot slot) {
		auto owner = weakOwner.lock();
		if (!owner)
			throw std::logic_error("Inventory is missing an owner");

		if (owner->getSide() == Side::Server)
			erase(slot);
		else if (auto player = std::dynamic_pointer_cast<Player>(owner))
			player->send(DropItemPacket(slot, true));
		else
			throw std::runtime_error("Only Players can discard items on the client");
	}

	bool Inventory::swap(Slot source, Slot destination) {
		if (slotCount <= source || slotCount <= destination || !storage.contains(source))
			return false;

		ItemStack &source_stack = storage.at(source);

		if (storage.contains(destination)) {
			ItemStack &destination_stack = storage.at(destination);
			if (destination_stack.canMerge(source_stack)) {
				ItemCount to_move = std::min(source_stack.count, destination_stack.item->maxCount - destination_stack.count);
				destination_stack.count += to_move;
				if ((source_stack.count -= to_move) == 0)
					storage.erase(source);
			} else
				std::swap(storage.at(source), storage.at(destination));
		} else {
			storage.emplace(destination, std::move(source_stack));
			storage.erase(source);
		}

		notifyOwner();
		return true;
	}

	void Inventory::erase(Slot slot, bool suppress_notification) {
		storage.erase(slot);
		if (!suppress_notification)
			notifyOwner();
	}

	void Inventory::erase(bool suppress_notification) {
		erase(activeSlot, suppress_notification);
	}

	ItemCount Inventory::count(const ItemID &id) const {
		if (id.getPath() == "attribute")
			return countAttribute(id);

		ItemCount out = 0;

		for (const auto &[slot, stored_stack]: storage)
			if (stored_stack.item->identifier == id)
				out += stored_stack.count;

		return out;
	}

	ItemCount Inventory::count(const Item &item) const {
		ItemCount out = 0;

		for (const auto &[slot, stored_stack]: storage)
			if (stored_stack.item->identifier == item.identifier)
				out += stored_stack.count;

		return out;
	}

	ItemCount Inventory::count(const ItemStack &stack) const {
		ItemCount out = 0;

		for (const auto &[slot, stored_stack]: storage)
			if (stack.canMerge(stored_stack))
				out += stored_stack.count;

		return out;
	}

	ItemCount Inventory::countAttribute(const Identifier &attribute) const {
		ItemCount out = 0;

		for (const auto &[slot, stack]: storage)
			if (stack.hasAttribute(attribute))
				out += stack.count;

		return out;
	}

	std::shared_ptr<Agent> Inventory::getOwner() const {
		if (auto owner = weakOwner.lock())
			return owner;
		throw std::runtime_error("Couldn't lock inventory owner");
	}

	ItemStack & Inventory::front() {
		if (storage.empty())
			throw std::out_of_range("Inventory empty");
		return storage.begin()->second;
	}

	const ItemStack & Inventory::front() const {
		if (storage.empty())
			throw std::out_of_range("Inventory empty");
		return storage.begin()->second;
	}

	ItemCount Inventory::remove(const ItemStack &stack_to_remove) {
		bool retry;
		ItemCount count_to_remove = stack_to_remove.count;
		ItemCount removed = 0;
		do {
			retry = false;
			for (auto &[slot, stack]: storage) {
				if (stack.canMerge(stack_to_remove)) {
					const auto to_remove = std::min(stack.count, count_to_remove);
					stack.count -= to_remove;
					count_to_remove -= to_remove;
					removed += 0;
					if (stack.count == 0) {
						// Erasing from a map can invalidate iterators.
						storage.erase(slot);
						retry = true;
						break;
					}

					if (count_to_remove == 0)
						return removed;
				}
			}

			if (count_to_remove == 0)
				return removed;
		} while (retry);

		return removed;
	}

	ItemCount Inventory::remove(const ItemStack &stack_to_remove, Slot slot) {
		auto &stack = storage.at(slot);
		if (!stack_to_remove.canMerge(stack))
			return 0;

		const auto to_remove = std::min(stack.count, stack_to_remove.count);
		if ((stack.count -= to_remove) == 0)
			storage.erase(slot);

		return to_remove;
	}

	ItemCount Inventory::remove(const CraftingRequirement &requirement) {
		if (requirement.is<ItemStack>())
			return remove(requirement.get<ItemStack>());
		return remove(requirement.get<AttributeRequirement>());
	}

	ItemCount Inventory::remove(const AttributeRequirement &requirement) {
		const Identifier &attribute = requirement.attribute;
		ItemCount count_remaining = requirement.count;
		ItemCount count_removed = 0;

		for (Slot slot = 0; slot < slotCount && 0 < count_remaining; ++slot) {
			if (auto *stack = (*this)[slot]; stack && stack->hasAttribute(attribute)) {
				const ItemCount to_remove = std::min(stack->count, count_remaining);
				stack->count  -= to_remove;
				count_removed += to_remove;
				if (0 == (count_remaining -= to_remove))
					break;
			}
		}

		compact();
		return count_removed;
	}

	bool Inventory::contains(Slot slot) const {
		return storage.contains(slot);
	}

	std::optional<Slot> Inventory::find(const ItemID &id) const {
		for (const auto &[slot, stack]: storage)
			if (stack.item->identifier == id)
				return slot;
		return std::nullopt;
	}

	std::optional<Slot> Inventory::findAttribute(const Identifier &attribute) const {
		for (const auto &[slot, stack]: storage)
			if (stack.item->attributes.contains(attribute))
				return slot;
		return std::nullopt;
	}

	ItemStack * Inventory::getActive() {
		return storage.contains(activeSlot)? &storage.at(activeSlot) : nullptr;
	}

	const ItemStack * Inventory::getActive() const {
		return storage.contains(activeSlot)? &storage.at(activeSlot) : nullptr;
	}

	void Inventory::setActive(Slot new_active, bool force) {
		if (!(0 <= new_active && new_active < slotCount)) {
			WARN("Bad slot: " << new_active << " (slot count: " << slotCount << ")");
			return;
		}

		auto owner = getOwner();
		auto player = std::dynamic_pointer_cast<Player>(owner);
		assert(player);
		const bool is_client = owner->getSide() == Side::Client;
		if (is_client && !force) {
			if (player)
				player->send(SetActiveSlotPacket(new_active));
		} else {
			activeSlot = new_active;
			if (is_client)
				notifyOwner();
			else if (player)
				player->send(ActiveSlotSetPacket(activeSlot));
		}
	}

	void Inventory::prevSlot() {
		if (0 < activeSlot)
			setActive(activeSlot - 1);
	}

	void Inventory::nextSlot() {
		if (activeSlot < slotCount - 1)
			setActive(activeSlot + 1);
	}

	ItemCount Inventory::craftable(const CraftingRecipe &recipe) const {
		ItemCount out = UINT64_MAX;

		for (const auto &input: recipe.input) {
			if (input.is<ItemStack>()) {
				const ItemStack &stack = input.get<ItemStack>();
				out = std::min(out, count(stack) / stack.count);
			} else {
				const auto &[attribute, attribute_count] = input.get<AttributeRequirement>();
				out = std::min(out, countAttribute(attribute) / attribute_count);
			}
		}

		return out;
	}

	bool Inventory::contains(const ItemStack &needle) const {
		ItemCount remaining = needle.count;
		for (const auto &[slot, stack]: storage) {
			if (!needle.canMerge(stack))
				continue;
			if (remaining <= stack.count)
				return true;
			remaining -= stack.count;
		}

		return false;
	}

	void Inventory::notifyOwner() {
		if (auto owner = weakOwner.lock()) {
			const auto side = owner->getRealm()->getSide();
			auto player = std::dynamic_pointer_cast<Player>(owner);

			if (side == Side::Server) {
				if (player)
					if (auto server_player = player->cast<ServerPlayer>())
						server_player->inventoryUpdated = true;
			} else {
				if (player)
					player->getRealm()->getGame().toClient().signal_player_inventory_update().emit(player);
				else
					owner->getRealm()->getGame().toClient().signal_other_inventory_update().emit(owner);
			}
		}
	}

	void Inventory::compact() {
		auto lock = storage.uniqueLock();

		for (auto iter = storage.begin(); iter != storage.end();) {
			if (iter->second.count == 0)
				storage.erase(iter++);
			else
				++iter;
		}
	}

	Inventory Inventory::fromJSON(Game &game, const nlohmann::json &json, const std::shared_ptr<Agent> &owner) {
		Inventory out(owner, 0);

		if (auto iter = json.find("storage"); iter != json.end())
			for (const auto &[key, val]: iter->items())
				out.storage.emplace(parseUlong(key), ItemStack::fromJSON(game, val));
		out.slotCount  = json.at("slotCount");
		out.activeSlot = json.at("activeSlot");

		return out;
	}

	template <>
	std::string Buffer::getType(const Inventory &) {
		return {'\xe1'};
	}

	Buffer & operator+=(Buffer &buffer, const Inventory &inventory) {
		buffer.appendType(inventory);
		if (auto locked = inventory.weakOwner.lock())
			buffer += locked->getGID();
		else
			buffer += static_cast<GlobalID>(-1);
		buffer += inventory.slotCount.load();
		buffer += inventory.activeSlot.load();
		{
			auto &storage = inventory.getStorage();
			auto lock = const_cast<decltype(Inventory().getStorage()) &>(storage).sharedLock();
			buffer += storage.getBase();
		}
		return buffer;
	}

	Buffer & operator<<(Buffer &buffer, const Inventory &inventory) {
		return buffer += inventory;
	}

	Buffer & operator>>(Buffer &buffer, Inventory &inventory) {
		const auto type = buffer.popType();
		if (!buffer.typesMatch(type, buffer.getType(inventory))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type) + ") in buffer (expected Inventory)");
		}
		const auto gid = popBuffer<GlobalID>(buffer);
		if (auto locked = inventory.weakOwner.lock())
			locked->setGID(gid);
		popBuffer(buffer, inventory.slotCount);
		popBuffer(buffer, inventory.activeSlot);
		inventory.setStorage(popBuffer<std::decay_t<decltype(inventory.getStorage())>>(buffer));
		return buffer;
	}

	void to_json(nlohmann::json &json, const Inventory &inventory) {
		for (const auto &[key, val]: inventory.storage)
			json["storage"][std::to_string(key)] = val;
		json["slotCount"]  = inventory.slotCount.load();
		json["activeSlot"] = inventory.activeSlot.load();
	}
}
