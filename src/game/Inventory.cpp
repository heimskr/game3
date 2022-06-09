#include <iostream>

#include "entity/Entity.h"
#include "entity/ItemEntity.h"
#include "game/Game.h"
#include "game/HasRealm.h"
#include "game/Inventory.h"
#include "game/Realm.h"

namespace Game3 {
	Inventory::Inventory(const std::shared_ptr<HasRealm> &owner_, Slot slot_count):
		owner(owner_), slotCount(slot_count) {}

	ItemStack * Inventory::operator[](size_t slot) {
		if (storage.contains(slot))
			return &storage.at(slot);
		return nullptr;
	}

	const ItemStack * Inventory::operator[](size_t slot) const {
		if (storage.contains(slot))
			return &storage.at(slot);
		return nullptr;
	}

	std::optional<ItemStack> Inventory::add(const ItemStack &stack) {
		int remaining = stack.count;

		for (auto &[slot, stored]: storage) {
			if (!stored.canMerge(stack))
				continue;
			const int storable = int(stored.item->maxCount) - int(stored.count);
			if (0 < storable) {
				const int to_store = std::min(remaining, storable);
				stored.count += to_store;
				remaining -= to_store;
				if (remaining <= 0)
					break;
			}
		}

		if (0 < remaining)
			for (Slot slot = 0; slot < slotCount; ++slot) {
				if (storage.contains(slot))
					continue;
				const unsigned to_store = std::min(unsigned(remaining), stack.item->maxCount);
				storage.emplace(slot, ItemStack(stack.item, to_store));
				remaining -= to_store;
				if (remaining <= 0)
					break;
			}

		notifyOwner();

		if (remaining < 0)
			throw std::logic_error("How'd we end up with " + std::to_string(remaining) + " items remaining?");

		if (remaining == 0)
			return std::nullopt;

		return ItemStack(stack.item, remaining);
	}

	void Inventory::drop(Slot slot) {
		if (!storage.contains(slot))
			return;

		auto locked_owner = owner.lock();
		if (!locked_owner)
			throw std::logic_error("Inventory is missing an owner");

		auto realm = locked_owner->getRealm();
		realm->spawn<ItemEntity>(locked_owner->getPosition(), storage.at(slot));
		storage.erase(slot);

		notifyOwner();
	}

	bool Inventory::swap(Slot source, Slot destination) {
		if (slotCount <= source || slotCount <= destination || !storage.contains(source))
			return false;

		if (storage.contains(destination)) {
			std::swap(storage.at(source), storage.at(destination));
		} else {
			storage[destination] = storage.at(source);
			storage.erase(source);
		}

		notifyOwner();
		return true;
	}

	void Inventory::erase(Slot slot) {
		storage.erase(slot);
		notifyOwner();
	}

	void Inventory::notifyOwner() {
		if (auto locked_owner = owner.lock())
			if (auto player = std::dynamic_pointer_cast<Player>(locked_owner))
				player->getRealm()->getGame().signal_player_inventory_update().emit(player);
	}

	Inventory Inventory::fromJSON(const nlohmann::json &json, const std::shared_ptr<HasRealm> &owner) {
		Inventory out(owner, 0);
		out.storage = json.at("storage");
		out.slotCount = json.at("slotCount");
		return out;
	}

	void to_json(nlohmann::json &json, const Inventory &inventory) {
		json["storage"] = inventory.storage;
		json["slotCount"] = inventory.slotCount;
	}
}
