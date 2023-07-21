#include "game/HasInventory.h"
#include "game/Inventory.h"
#include "pipes/ItemNetwork.h"
#include "realm/Realm.h"

namespace Game3 {
	void ItemNetwork::tick(Tick tick_id) {
		if (!canTick(tick_id))
			return;

		PipeNetwork::tick(tick_id);

		auto realm = weakRealm.lock();
		if (!realm)
			return;

		// Doing this instead of std::erase_if so that perhaps later I could do something special on each removal.
		std::vector<PairSet::iterator> to_erase;

		for (auto iter = extractions.begin(), end = extractions.end(); iter != end; ++iter) {
			const auto &[position, direction] = *iter;

			auto tile_entity = realm->tileEntityAt(position);
			if (!tile_entity) {
				to_erase.push_back(iter);
				continue;
			}

			auto &rr_iter = *roundRobinIterator;

			if (rr_iter == insertions.end())
				break;

			if (auto has_inventory = tile_entity->cast<HasInventory>(); has_inventory && has_inventory->inventory) {
				auto &inventory = *has_inventory->inventory;
				if (!inventory.empty()) {
					auto &storage = inventory.getStorage();
					auto lock = storage.uniqueLock();
					auto &[slot, stack] = *storage.begin();

					std::optional<ItemStack> leftover = getRoundRobinInventory()->add(stack);

					if (leftover) {
						// If the insertion didn't fully complete, we need to try to put the leftovers
						// in the next inventories in the round-robin configuration.
						ItemStack original_leftover = *leftover;
						auto old_iter = rr_iter;
						// Keep trying to insert into the next machine until we reach
						// the original machine or the leftovers are all gone.
						do {
							advanceRoundRobin();
							// Prevent insertion to self. It would presumably cause issues with locks.
							if (rr_iter->first == tile_entity->getPosition())
								continue;
							leftover = getRoundRobinInventory()->add(*leftover);
						} while (old_iter != rr_iter && leftover);

						if (leftover) {
							// If there's anything left over, subtract the amount that was inserted elsewhere
							// from the original stack extracted from.
							stack.count -= original_leftover.count - leftover->count;
							inventory.notifyOwner();
						} else {
							// Otherwise, we've sent out the entire stack and can simply erase it.
							inventory.erase(slot, false);
						}
					} else {
						// If the insertion completed successfully without leftovers, erase the source slot.
						inventory.erase(slot, false);
					}
				}
			}
		}

		for (const auto &iter: to_erase)
			extractions.erase(iter);
	}

	void ItemNetwork::advanceRoundRobin() {
		if (!roundRobinIterator || *roundRobinIterator == insertions.end() || ++*roundRobinIterator == insertions.end())
			roundRobinIterator = insertions.begin();

		cachedRoundRobinInventory = nullptr;
	}

	std::shared_ptr<Inventory> ItemNetwork::getRoundRobinInventory() {
		if (cachedRoundRobinInventory)
			return cachedRoundRobinInventory;

		if (!roundRobinIterator)
			roundRobinIterator = insertions.begin();

		if (*roundRobinIterator == insertions.end())
			return nullptr;

		auto realm = weakRealm.lock();
		if (!realm)
			return nullptr;

		TileEntityPtr tile_entity = realm->tileEntityAt((*roundRobinIterator)->first);
		if (!tile_entity)
			return nullptr;

		std::shared_ptr has_inventory = tile_entity->cast<HasInventory>();
		if (!has_inventory)
			return nullptr;

		return cachedRoundRobinInventory = has_inventory->inventory;
	}
}
