#include <cassert>

#include "Log.h"
#include "entity/Entity.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "entity/ServerPlayer.h"
#include "game/Agent.h"
#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "item/Item.h"
#include "net/Buffer.h"
#include "packet/InventoryPacket.h"
#include "packet/SetActiveSlotPacket.h"
#include "packet/DropItemPacket.h"
#include "realm/Realm.h"
#include "recipe/CraftingRecipe.h"
#include "tileentity/InventoriedTileEntity.h"
#include "util/Util.h"

namespace Game3 {
	ServerInventory::ServerInventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot, Storage storage_):
		StorageInventory(std::move(owner), slot_count, active_slot, std::move(storage_)) {}

	std::unique_ptr<Inventory> ServerInventory::copy() const {
		return std::make_unique<ServerInventory>(*this);
	}

	std::optional<ItemStack> ServerInventory::add(const ItemStack &stack, const std::function<bool(Slot)> &predicate, Slot start) {
		ssize_t remaining = stack.count;

		if (0 <= start) {
			if (slotCount <= start)
				throw std::out_of_range("Can't start at slot " + std::to_string(start) + ": out of range");
			if (storage.contains(start) && predicate(start)) {
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
				assert(storage.try_emplace(start, stack.getGame(), stack.item, to_store, stack.data).second);
				remaining -= to_store;
			}
		}

		if (0 < remaining) {
			for (auto &[slot, stored]: storage) {
				if (slot == start || !stored.canMerge(stack) || !predicate(slot))
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
			for (Slot slot = 0; slot < slotCount; ++slot) {
				if (storage.contains(slot) || !predicate(slot))
					continue;
				const ItemCount to_store = std::min(ItemCount(remaining), stack.item->maxCount);
				storage.emplace(slot, ItemStack(stack.getGame(), stack.item, to_store, stack.data));
				remaining -= to_store;
				if (remaining <= 0)
					break;
			}
		}

		if (remaining != ssize_t(stack.count))
			notifyOwner();

		if (remaining < 0 || ssize_t(stack.count) < remaining)
			throw std::logic_error("How'd we end up with " + std::to_string(remaining) + " items remaining?");

		if (remaining == 0)
			return std::nullopt;

		return ItemStack(getOwner()->getRealm()->getGame(), stack.item, remaining, stack.data);
	}

	void ServerInventory::drop(Slot slot) {
		if (!storage.contains(slot))
			return;

		auto owner = weakOwner.lock();
		if (!owner)
			throw std::logic_error("ServerInventory is missing an owner");

		auto realm = owner->getRealm();
		realm->spawn<ItemEntity>(owner->getPosition(), storage.at(slot));
		erase(slot);
		notifyOwner();
	}

	void ServerInventory::discard(Slot slot) {
		erase(slot);
		notifyOwner();
	}

	void ServerInventory::swap(Slot source, Slot destination) {
		if (slotCount <= source || slotCount <= destination || !storage.contains(source))
			return;

		ItemStack &source_stack = storage.at(source);

		std::function<void()> action;

		if (onSwap)
			action = onSwap(*this, source, *this, destination);

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

		if (action)
			action();

		notifyOwner();
	}

	void ServerInventory::erase(Slot slot) {
		std::function<void()> after;
		if (onRemove)
			after = onRemove(slot);
		storage.erase(slot);
		if (after)
			after();
	}

	ItemCount ServerInventory::remove(const ItemStack &stack_to_remove) {
		// Could just use a simple call to remove(const ItemStack &, const std::function<bool(Slot)> &) with [](Slot) { return true; },
		// but that would be slightly slower so we'll just spam a near copy of that function here.

		ItemCount count_to_remove = stack_to_remove.count;
		ItemCount removed = 0;

		std::erase_if(storage, [&](auto &item) {
			if (count_to_remove == 0)
				return false;

			auto &[slot, stack] = item;

			if (stack.canMerge(stack_to_remove)) {
				const ItemCount to_remove = std::min(stack.count, count_to_remove);
				// The const_cast is for certain older compilers (looking at you, gcc) that constify the first argument of the function passed to std::erase_if.
				// Such behavior doesn't comply with the C++ standard, but I want to support those compilers anyway.
				const_cast<ItemStack &>(stack).count -= to_remove;
				count_to_remove -= to_remove;
				removed += to_remove;

				if (stack.count == 0)
					return true;
			}

			return false;
		});

		if (0 < removed)
			notifyOwner();

		return removed;
	}

	ItemCount ServerInventory::remove(const ItemStack &stack_to_remove, const std::function<bool(Slot)> &predicate) {
		ItemCount count_to_remove = stack_to_remove.count;
		ItemCount removed = 0;

		std::erase_if(storage, [&](auto &item) {
			if (count_to_remove == 0)
				return false;

			auto &[slot, stack] = item;

			if (predicate(slot) && stack.canMerge(stack_to_remove)) {
				const ItemCount to_remove = std::min(stack.count, count_to_remove);
				// See above for the reasoning behind this const_cast.
				const_cast<ItemStack &>(stack).count -= to_remove;
				count_to_remove -= to_remove;
				removed += to_remove;

				if (stack.count == 0)
					return true;
			}

			return false;
		});

		if (0 < removed)
			notifyOwner();

		return removed;
	}

	ItemCount ServerInventory::remove(const ItemStack &stack_to_remove, Slot slot) {
		auto iter = storage.find(slot);
		if (iter == storage.end()) {
			if (auto owner = weakOwner.lock())
				WARN("Trying to remove from empty slot " << slot << " in " << owner->getName() << ' ' << owner->getGID() << "'s inventory");
			else
				WARN("Trying to remove from empty slot " << slot << " in an unowned inventory");
			return 0;
		}

		auto &stack = iter->second;
		if (!stack_to_remove.canMerge(stack))
			return 0;

		const ItemCount to_remove = std::min(stack.count, stack_to_remove.count);
		if ((stack.count -= to_remove) == 0)
			storage.erase(slot);

		notifyOwner();
		return to_remove;
	}

	ItemCount ServerInventory::remove(const CraftingRequirement &requirement, const ConstPredicate &predicate) {
		if (requirement.is<ItemStack>())
			return remove(requirement.get<ItemStack>(), predicate);
		return remove(requirement.get<AttributeRequirement>(), predicate);
	}

	ItemCount ServerInventory::remove(const AttributeRequirement &requirement, const ConstPredicate &predicate) {
		const Identifier &attribute = requirement.attribute;
		ItemCount count_remaining = requirement.count;
		ItemCount count_removed = 0;

		for (Slot slot = 0; slot < slotCount && 0 < count_remaining; ++slot) {
			if (auto *stack = (*this)[slot]; stack && predicate(*stack, slot) && stack->hasAttribute(attribute)) {
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

	void ServerInventory::setActive(Slot new_active, bool) {
		if (!(0 <= new_active && new_active < slotCount)) {
			WARN("Bad slot: " << new_active << " (slot count: " << slotCount << ")");
			return;
		}

		auto owner = getOwner();
		auto player = std::dynamic_pointer_cast<Player>(owner);
		assert(player);
		activeSlot = new_active;
		player->send(SetActiveSlotPacket(new_active));
		notifyOwner();
	}

	void ServerInventory::notifyOwner() {
		if (suppressInventoryNotifications)
			return;

		if (auto owner = weakOwner.lock()) {
			owner->increaseUpdateCounter();
			if (auto tile_entity = std::dynamic_pointer_cast<InventoriedTileEntity>(owner))
				tile_entity->queueBroadcast();
			else if (auto server_player = std::dynamic_pointer_cast<ServerPlayer>(owner))
				server_player->inventoryUpdated = true;
		}
	}

	ServerInventory ServerInventory::fromJSON(Game &game, const nlohmann::json &json, const std::shared_ptr<Agent> &owner) {
		ServerInventory out(owner, 0);

		if (auto iter = json.find("storage"); iter != json.end())
			for (const auto &[key, val]: iter->items())
				out.storage.emplace(parseUlong(key), ItemStack::fromJSON(game, val));
		out.slotCount  = json.at("slotCount");
		out.activeSlot = json.at("activeSlot");

		return out;
	}

	template <>
	std::string Buffer::getType(const ServerInventory &) {
		return "\xe1";
	}

	Buffer & operator+=(Buffer &buffer, const ServerInventory &inventory) {
		buffer.appendType(inventory);
		if (auto locked = inventory.weakOwner.lock())
			buffer << locked->getGID();
		else
			buffer << static_cast<GlobalID>(-1);
		buffer << inventory.slotCount.load();
		buffer << inventory.activeSlot.load();
		{
			auto &storage = inventory.getStorage();
			auto lock = storage.sharedLock();
			buffer << storage.getBase();
		}
		return buffer;
	}

	Buffer & operator<<(Buffer &buffer, const ServerInventory &inventory) {
		return buffer += inventory;
	}

	Buffer & operator>>(Buffer &buffer, ServerInventory &inventory) {
		const auto type = buffer.popType();
		if (!buffer.typesMatch(type, buffer.getType(inventory))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected inventory)");
		}
		const auto gid = buffer.take<GlobalID>();
		if (auto locked = inventory.weakOwner.lock())
			locked->setGID(gid);
		inventory.slotCount = buffer.take<Slot>();
		inventory.activeSlot = buffer.take<Slot>();
		inventory.setStorage(buffer.take<std::decay_t<decltype(inventory.getStorage())>>());
		return buffer;
	}

	void to_json(nlohmann::json &json, const ServerInventory &inventory) {
		for (const auto &[key, val]: inventory.getStorage())
			json["storage"][std::to_string(key)] = val;
		json["slotCount"]  = inventory.slotCount.load();
		json["activeSlot"] = inventory.activeSlot.load();
	}
}
