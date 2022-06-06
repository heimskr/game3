#include <iostream>

#include "entity/Entity.h"
#include "entity/ItemEntity.h"
#include "game/Inventory.h"
#include "game/Realm.h"

namespace Game3 {
	Inventory::Inventory(const std::shared_ptr<Entity> &owner_, Slot slot_count):
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

	std::unique_ptr<ItemStack> Inventory::add(const ItemStack &stack) {
		int remaining = stack.count;

		for (auto &[slot, stored]: storage) {
			if (!stored.canMerge(stack))
				continue;
			const int storable = int(stored.item->maxCount) - int(stored.count);
			if (0 < storable) {
				const int to_store = std::min(remaining, storable);
				stored.count += to_store;
				remaining -= to_store;
				if (remaining == 0)
					break;
			}
		}

		for (Slot slot = 0; slot < slotCount; ++slot) {
			if (storage.contains(slot))
				continue;
			const unsigned to_store = std::min(unsigned(remaining), stack.item->maxCount);
			storage.emplace(slot, ItemStack(stack.item, to_store));
			remaining -= to_store;
			if (remaining == 0)
				break;
		}

		return remaining == 0? nullptr : std::make_unique<ItemStack>(stack.item, remaining);
	}

	void Inventory::drop(Slot slot) {
		if (!storage.contains(slot))
			return;

		auto entity = owner.lock();
		if (!entity)
			throw std::logic_error("Inventory is missing an owner");

		auto realm = entity->weakRealm.lock();
		if (!realm)
			throw std::logic_error("Inventory owner has no realm");

		std::cout << "Spawning ItemEntity at " << entity->position << "\n";

		realm->spawn<ItemEntity>(entity->position, storage.at(slot));
		storage.erase(slot);
	}

	Inventory Inventory::fromJSON(const nlohmann::json &json, const std::shared_ptr<Entity> &owner) {
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
