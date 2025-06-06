#include "entity/Entity.h"
#include "entity/ItemEntity.h"
#include "entity/Player.h"
#include "entity/ServerPlayer.h"
#include "game/Agent.h"
#include "game/ClientGame.h"
#include "game/ServerInventory.h"
#include "item/Item.h"
#include "net/Buffer.h"
#include "packet/DropItemPacket.h"
#include "packet/InventoryPacket.h"
#include "packet/SetActiveSlotPacket.h"
#include "realm/Realm.h"
#include "recipe/CraftingRecipe.h"
#include "tileentity/InventoriedTileEntity.h"
#include "util/Cast.h"
#include "util/Log.h"
#include "util/Util.h"

#include <cassert>

namespace Game3 {
	ServerInventory::ServerInventory(std::shared_ptr<Agent> owner, Slot slot_count, Slot active_slot, InventoryID index_, Storage storage_):
		StorageInventory(std::move(owner), slot_count, active_slot, index_, std::move(storage_)) {}

	std::unique_ptr<Inventory> ServerInventory::copy() const {
		auto out = std::make_unique<ServerInventory>(*this);
		for (auto &[slot, stack]: out->storage) {
			if (stack) {
				stack = stack->copy();
			}
		}
		return out;
	}

	ItemStackPtr ServerInventory::add(const ItemStackPtr &stack, const std::function<bool(Slot)> &predicate, Slot start) {
		ssize_t remaining = stack->count;

		if (0 <= start) {
			if (slotCount <= start) {
				throw std::out_of_range(std::format("Can't start at slot {}: out of range", start));
			}

			if (auto iter = storage.find(start); iter != storage.end() && predicate(start)) {
				ItemStackPtr stored = iter->second;
				if (stored->canMerge(*stack)) {
					const ssize_t storable = ssize_t(stored->item->maxCount) - ssize_t(stored->count);
					if (0 < storable) {
						const ItemCount to_store = std::min(ItemCount(remaining), ItemCount(storable));
						stored->count += to_store;
						remaining -= to_store;
					}
				}
			} else {
				const ItemCount to_store = std::min(ItemCount(stack->item->maxCount), ItemCount(remaining));
				const bool emplaced = storage.try_emplace(start, ItemStack::create(stack->getGame(), stack->item, to_store, stack->data)).second;
				assert(emplaced);
				remaining -= to_store;
			}
		}

		if (0 < remaining) {
			for (auto &[slot, stored]: storage) {
				if (slot == start || !stored->canMerge(*stack) || !predicate(slot)) {
					continue;
				}
				const ssize_t storable = ssize_t(stored->item->maxCount) - ssize_t(stored->count);
				if (0 < storable) {
					const ItemCount to_store = std::min<ItemCount>(remaining, storable);
					stored->count += to_store;
					remaining -= to_store;
					if (remaining <= 0) {
						break;
					}
				}
			}
		}

		if (0 < remaining) {
			for (Slot slot = 0; slot < slotCount; ++slot) {
				if (storage.contains(slot) || !predicate(slot)) {
					continue;
				}
				const ItemCount to_store = std::min(static_cast<ItemCount>(remaining), stack->item->maxCount);
				storage.emplace(slot, ItemStack::create(stack->getGame(), stack->item, to_store, stack->data));
				remaining -= to_store;
				if (remaining <= 0) {
					break;
				}
			}
		}

		if (remaining != static_cast<ssize_t>(stack->count)) {
			notifyOwner(stack);
		}

		if (remaining < 0 || static_cast<ssize_t>(stack->count) < remaining) {
			throw std::logic_error("How'd we end up with " + std::to_string(remaining) + " items remaining?");
		}

		if (remaining == 0) {
			return nullptr;
		}

		return ItemStack::create(stack->hasGame() ? stack->getGame() : getOwner()->getRealm()->getGame(), stack->item, remaining, stack->data);
	}

	void ServerInventory::drop(Slot slot) {
		if (!storage.contains(slot)) {
			return;
		}

		auto owner = weakOwner.lock();
		if (!owner) {
			throw std::logic_error("ServerInventory is missing an owner");
		}

		auto realm = owner->getRealm();
		realm->spawn<ItemEntity>(owner->getPosition(), storage.at(slot));
		erase(slot);
		notifyOwner({});
	}

	void ServerInventory::discard(Slot slot) {
		erase(slot);
		notifyOwner({});
	}

	void ServerInventory::swap(Slot source, Slot destination) {
		if (slotCount <= source || slotCount <= destination || !storage.contains(source)) {
			return;
		}

		ItemStackPtr source_stack = storage.at(source);

		std::function<void()> action;

		if (onSwap) {
			action = onSwap(*this, source, *this, destination);
		}

		if (storage.contains(destination)) {
			ItemStackPtr destination_stack = storage.at(destination);
			if (destination_stack->canMerge(*source_stack)) {
				ItemCount to_move = std::min(source_stack->count, destination_stack->item->maxCount - destination_stack->count);
				destination_stack->count += to_move;
				if ((source_stack->count -= to_move) == 0) {
					storage.erase(source);
				}
			} else {
				std::swap(storage.at(source), storage.at(destination));
			}
		} else {
			storage.emplace(destination, std::move(source_stack));
			storage.erase(source);
		}

		if (action) {
			action();
		}

		notifyOwner({});
	}

	void ServerInventory::erase(Slot slot) {
		if (slot < 0) {
			throw std::invalid_argument("Can't erase invalid slot " + std::to_string(slot));
		}

		std::function<void()> after;
		if (onRemove) {
			after = onRemove(slot);
		}
		storage.erase(slot);
		if (after) {
			after();
		}
	}

	void ServerInventory::clear() {
		// TODO: some kind of callback?
		storage.clear();
	}

	ItemCount ServerInventory::remove(const ItemStackPtr &stack_to_remove) {
		// Could just use a simple call to remove(const ItemStackPtr &, const SlotPredicate &) with [](Slot) { return true; },
		// but that would be slightly slower so we'll just spam a near copy of that function here.

		ItemCount count_to_remove = stack_to_remove->count;
		ItemCount removed = 0;

		std::erase_if(storage, [&](auto &item) {
			if (count_to_remove == 0) {
				return false;
			}

			auto &[slot, stack] = item;

			if (stack->canMerge(*stack_to_remove)) {
				const ItemCount to_remove = std::min(stack->count, count_to_remove);
				stack->count -= to_remove;
				count_to_remove -= to_remove;
				removed += to_remove;

				if (stack->count == 0) {
					return true;
				}
			}

			return false;
		});

		if (0 < removed) {
			notifyOwner({});
		}

		return removed;
	}

	ItemCount ServerInventory::remove(const ItemStackPtr &stack_to_remove, const Predicate &predicate) {
		ItemCount count_to_remove = stack_to_remove->count;
		ItemCount removed = 0;

		std::erase_if(storage, [&](auto &item) {
			if (count_to_remove == 0) {
				return false;
			}

			auto &[slot, stack] = item;

			if (predicate(stack, slot) && stack->canMerge(*stack_to_remove)) {
				const ItemCount to_remove = std::min(stack->count, count_to_remove);
				stack->count -= to_remove;
				count_to_remove -= to_remove;
				removed += to_remove;

				if (stack->count == 0) {
					return true;
				}
			}

			return false;
		});

		if (0 < removed) {
			notifyOwner({});
		}

		return removed;
	}

	ItemCount ServerInventory::remove(const ItemStackPtr &stack_to_remove, Slot slot) {
		auto iter = storage.find(slot);
		if (iter == storage.end()) {
			if (auto owner = weakOwner.lock()) {
				WARN("Trying to remove from empty slot {} in {} {}'s inventory", slot, owner->getName(), owner->getGID());
			} else {
				WARN("Trying to remove from empty slot {} in an unowned inventory", slot);
			}
			return 0;
		}

		ItemStackPtr &stack = iter->second;
		if (!stack_to_remove->canMerge(*stack)) {
			return 0;
		}

		const ItemCount to_remove = std::min(stack->count, stack_to_remove->count);
		if ((stack->count -= to_remove) == 0) {
			storage.erase(slot);
		}

		notifyOwner({});
		return to_remove;
	}

	ItemCount ServerInventory::remove(const CraftingRequirement &requirement, const Predicate &predicate) {
		if (requirement.is<ItemStackPtr>()) {
			return remove(requirement.get<ItemStackPtr>(), predicate);
		}
		return remove(requirement.get<AttributeRequirement>(), predicate);
	}

	ItemCount ServerInventory::remove(const AttributeRequirement &requirement, const Predicate &predicate) {
		const Identifier &attribute = requirement.attribute;
		ItemCount count_remaining = requirement.count;
		ItemCount count_removed = 0;

		for (Slot slot = 0; slot < slotCount && 0 < count_remaining; ++slot) {
			if (ItemStackPtr stack = (*this)[slot]; stack && predicate(stack, slot) && stack->hasAttribute(attribute)) {
				const ItemCount to_remove = std::min(stack->count, count_remaining);
				stack->count -= to_remove;
				count_removed += to_remove;
				if (0 == (count_remaining -= to_remove)) {
					break;
				}
			}
		}

		compact();
		return count_removed;
	}

	void ServerInventory::setActive(Slot new_active, bool) {
		if (!(0 <= new_active && new_active < slotCount)) {
			WARN("Bad slot: {} (slot count: {})", new_active, slotCount);
			return;
		}

		auto owner = getOwner();
		auto player = safeDynamicCast<Player>(owner);
		activeSlot = new_active;
		player->send(make<SetActiveSlotPacket>(new_active));
		notifyOwner({});
	}

	void ServerInventory::notifyOwner(std::optional<std::variant<ItemStackPtr, Slot>> variant) {
		AgentPtr owner = weakOwner.lock();

		if (owner) {
			owner->inventoryUpdated(index);

			if (suppressInventoryNotifications) {
				return;
			}

			owner->increaseUpdateCounter();
			if (auto tile_entity = std::dynamic_pointer_cast<InventoriedTileEntity>(owner)) {
				tile_entity->queueBroadcast();
			} else if (auto server_player = std::dynamic_pointer_cast<ServerPlayer>(owner)) {
				server_player->inventoryUpdated = true;
				if (variant) {
					if (std::holds_alternative<Slot>(*variant)) {
						if (ItemStackPtr stack = (*this)[std::get<Slot>(*variant)]) {
							server_player->addKnownItem(stack);
						}
					} else if (ItemStackPtr stack = std::get<ItemStackPtr>(*variant)) {
						server_player->addKnownItem(stack);
					}
				}
			}
		}
	}

	ServerInventory tag_invoke(boost::json::value_to_tag<ServerInventory>, const boost::json::value &json, const std::pair<GamePtr, AgentPtr> &pair) {
		const auto &[game, owner] = pair;

		ServerInventory out(owner, 0);

		const auto &object = json.as_object();

		if (auto iter = object.find("storage"); iter != object.end()) {
			for (const auto &[key, val]: iter->value().as_object()) {
				out.storage.emplace(parseUlong(static_cast<std::string_view>(key)), boost::json::value_to<ItemStackPtr>(val, game));
			}
		}

		out.slotCount = boost::json::value_to<Slot>(object.at("slotCount"));
		out.activeSlot = boost::json::value_to<Slot>(object.at("activeSlot"));

		return out;
	}

	template <>
	std::string Buffer::getType(const ServerInventory &, bool) {
		return "\xe1";
	}

	Buffer & operator+=(Buffer &buffer, const ServerInventory &inventory) {
		buffer.appendType(inventory, false);
		if (inventory.hasOwner()) {
			buffer << inventory.getOwner()->getGID();
		} else {
			buffer << static_cast<GlobalID>(-1);
		}
		buffer << inventory.getSlotCount();
		buffer << inventory.activeSlot.load();
		buffer << inventory.index.load();
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
		if (!buffer.typesMatch(type, buffer.getType(inventory, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected inventory)");
		}
		const auto gid = buffer.take<GlobalID>();
		if (inventory.hasOwner()) {
			inventory.getOwner()->setGID(gid);
		}
		inventory.setSlotCount(buffer.take<Slot>());
		inventory.activeSlot = buffer.take<Slot>();
		inventory.index = buffer.take<InventoryID>();
		inventory.setStorage(buffer.take<std::decay_t<decltype(inventory.getStorage())>>());
		return buffer;
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const ServerInventory &inventory) {
		auto &object = json.emplace_object();

		auto &storage = object["storage"].as_object();
		for (const auto &[key, val]: inventory.getStorage()) {
			storage[std::to_string(key)] = boost::json::value_from(*val);
		}

		object["slotCount"] = inventory.getSlotCount();
		object["activeSlot"] = inventory.activeSlot.load();
	}
}
