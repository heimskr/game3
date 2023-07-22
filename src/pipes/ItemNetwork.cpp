#include "Log.h"
#include "game/HasInventory.h"
#include "game/StorageInventory.h"
#include "pipes/ItemNetwork.h"
#include "realm/Realm.h"

namespace Game3 {
	void ItemNetwork::tick(Tick tick_id) {
		if (!canTick(tick_id))
			return;

		PipeNetwork::tick(tick_id);

		auto realm = weakRealm.lock();
		if (!realm || insertions.empty())
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

			if (!roundRobinIterator)
				advanceRoundRobin();

			auto &rr_iter = *roundRobinIterator;

			if (auto has_inventory = tile_entity->cast<HasInventory>(); has_inventory && has_inventory->inventory) {
				auto &inventory = *has_inventory->inventory;
				if (!inventory.empty()) {
					auto &storage = dynamic_cast<StorageInventory &>(inventory).getStorage();
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
						} else {
							// Otherwise, we've sent out the entire stack and can simply erase it.
							inventory.erase(slot);
						}
					} else {
						// If the insertion completed successfully without leftovers, erase the source slot.
						inventory.erase(slot);
					}

					inventory.notifyOwner();
				}
			}
		}

		for (const auto &iter: to_erase)
			extractions.erase(iter);
	}

	void ItemNetwork::advanceRoundRobin() {
		auto realm = weakRealm.lock();
		if (!realm) {
			roundRobinIterator = insertions.end();
			return;
		}

		if (insertions.size() == 1) {
			auto tile_entity = realm->tileEntityAt(insertions.begin()->first);

			if (std::dynamic_pointer_cast<HasInventory>(tile_entity))
				roundRobinIterator = insertions.begin();
			else
				roundRobinIterator = insertions.end();

			return;
		}

		if (!roundRobinIterator || *roundRobinIterator == insertions.end())
			roundRobinIterator = insertions.begin();

		auto old_iter = *roundRobinIterator;
		bool has_inventory = false;

		// Keep searching for the next insertion that has an inventory until we reach the initial iterator.
		do {
			if (++*roundRobinIterator == insertions.end())
				roundRobinIterator = insertions.begin();
			has_inventory = std::dynamic_pointer_cast<HasInventory>(realm->tileEntityAt((*roundRobinIterator)->first)) != nullptr;
		} while (*roundRobinIterator != old_iter && !has_inventory);

		// If we haven't found an inventoried tile entity by this point, it means none exists among the insertion set;
		// therefore, we need to invalidate the iterator.
		if (!has_inventory)
			roundRobinIterator = insertions.end();

		cachedRoundRobinInventory = nullptr;
	}

	std::shared_ptr<Inventory> ItemNetwork::getRoundRobinInventory() {
		if (cachedRoundRobinInventory)
			return cachedRoundRobinInventory;

		if (!roundRobinIterator)
			advanceRoundRobin();

		if (*roundRobinIterator == insertions.end())
			return nullptr;

		auto realm = weakRealm.lock();
		if (!realm)
			return nullptr;

		TileEntityPtr tile_entity = realm->tileEntityAt((*roundRobinIterator)->first);
		if (!tile_entity)
			return nullptr;

		auto has_inventory = std::dynamic_pointer_cast<HasInventory>(tile_entity);
		if (!has_inventory)
			return nullptr;

		return cachedRoundRobinInventory = has_inventory->inventory;
	}
}
