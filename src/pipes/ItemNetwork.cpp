#include "Log.h"
#include "game/HasInventory.h"
#include "game/StorageInventory.h"
#include "pipes/ItemNetwork.h"
#include "realm/Realm.h"
#include "tileentity/InventoriedTileEntity.h"

namespace Game3 {
	void ItemNetwork::tick(Tick tick_id) {
		if (!canTick(tick_id))
			return;

		PipeNetwork::tick(tick_id);

		auto realm = weakRealm.lock();
		if (!realm || insertions.empty())
			return;

		// Every so often, if there's anything in the overflowQueue, we try to insert that somewhere instead of extracting anything more.
		if (overflowPeriod != 0 && tick_id % overflowPeriod == 0 && !overflowQueue.empty()) {
			auto lock = overflowQueue.uniqueLock();
			std::optional<ItemStack> stack = std::move(overflowQueue.front());
			overflowQueue.pop_front();

			if (!roundRobinIterator)
				advanceRoundRobin();

			auto &round_robin_iter = *roundRobinIterator;
			const auto old_iter = round_robin_iter;

			do {
				advanceRoundRobin();
				if (InventoryPtr round_robin_inventory = getRoundRobinInventory())
					stack = round_robin_inventory->add(*stack);
				else
					break;
			} while (old_iter != round_robin_iter && stack);

			// If the stack couldn't be fully inserted, put the remainder at the end of the overflow queue.
			if (stack)
				overflowQueue.push_back(std::move(*stack));

			return;
		}

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

			auto &round_robin_iter = *roundRobinIterator;

			if (round_robin_iter == insertions.end())
				continue;

			if (auto inventoried = tile_entity->cast<InventoriedTileEntity>(); inventoried && !inventoried->empty()) {
				std::optional<ItemStack> extracted = inventoried->extractItem(direction, true);
				if (!extracted)
					continue;

				std::optional<ItemStack> leftover;

				if (InventoryPtr round_robin_inventory = getRoundRobinInventory()) {
					leftover = round_robin_inventory->add(*extracted);
				} else {
					overflowQueue.push_back(std::move(*extracted));
					return;
				}

				if (leftover) {
					// If the insertion didn't fully complete, we need to try to put the leftovers
					// in the next inventories in the round-robin configuration.
					const auto old_iter = round_robin_iter;

					// Keep trying to insert into the next machine until we reach
					// the original machine or the leftovers are all gone.
					do {
						advanceRoundRobin();

						// Prevent insertion to self. It would presumably cause issues with locks.
						if (round_robin_iter->first == tile_entity->getPosition())
							continue;

						if (InventoryPtr round_robin_inventory = getRoundRobinInventory()) {
							leftover = round_robin_inventory->add(*leftover);
						} else {
							overflowQueue.push_back(std::move(*leftover));
							return;
						}
					} while (old_iter != round_robin_iter && leftover);

					if (leftover) {
						// If there's anything left over, move it to the overflowQueue so we can try to insert it somewhere another time.
						overflowQueue.push_back(std::move(*leftover));
					}
				} else {
					advanceRoundRobin();
				}
			}
		}

		for (const auto &iter: to_erase)
			extractions.erase(iter);
	}

	void ItemNetwork::lastPipeRemoved(Position where) {
		if (overflowQueue.empty())
			return;

		RealmPtr realm = weakRealm.lock();
		if (!realm) {
			WARN("Item network destroyed while unable to drop overflow");
			return;
		}

		for (const ItemStack &stack: overflowQueue)
			stack.spawn(realm, where);

		// Just in case.
		overflowQueue.clear();
	}

	bool ItemNetwork::canWorkWith(const std::shared_ptr<TileEntity> &tile_entity) const {
		return std::dynamic_pointer_cast<InventoriedTileEntity>(tile_entity) != nullptr;
	}

	void ItemNetwork::advanceRoundRobin() {
		RealmPtr realm = weakRealm.lock();
		if (!realm) {
			roundRobinIterator = insertions.end();
			return;
		}

		if (insertions.size() == 1) {
			TileEntityPtr tile_entity = realm->tileEntityAt(insertions.begin()->first);

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
	}

	InventoryPtr ItemNetwork::getRoundRobinInventory() {
		if (!roundRobinIterator)
			advanceRoundRobin();

		if (*roundRobinIterator == insertions.end())
			return nullptr;

		RealmPtr realm = weakRealm.lock();
		if (!realm)
			return nullptr;

		TileEntityPtr tile_entity = realm->tileEntityAt((*roundRobinIterator)->first);
		if (!tile_entity)
			return nullptr;

		auto has_inventory = std::dynamic_pointer_cast<HasInventory>(tile_entity);
		if (!has_inventory)
			return nullptr;

		return has_inventory->inventory;
	}
}
