#include "Log.h"
#include "net/Buffer.h"

#include "game/HasInventory.h"
#include "game/StorageInventory.h"
#include "pipes/ItemFilter.h"
#include "pipes/ItemNetwork.h"
#include "realm/Realm.h"
#include "tileentity/InventoriedTileEntity.h"
#include "tileentity/Pipe.h"

namespace Game3 {
	void ItemNetwork::tick(const std::shared_ptr<Game> &game, Tick tick_id) {
		if (!canTick(tick_id)) {
			PipeNetwork::tick(game, tick_id);
			return;
		}

		PipeNetwork::tick(game, tick_id);

		auto this_lock = uniqueLock();

		auto insertions_lock = insertions.uniqueLock();
		RealmPtr realm = weakRealm.lock();
		{
			if (!realm || insertions.empty())
				return;
		}

		auto overflow_lock = overflowQueue.uniqueLock();
		auto round_robin_lock = roundRobinIterator.uniqueLock();

		// Every so often, if there's anything in the overflowQueue, we try to insert that somewhere instead of extracting anything more.
		if (overflowPeriod != 0 && tick_id % overflowPeriod == 0 && !overflowQueue.empty()) {
			ItemStackPtr stack = std::move(overflowQueue.front());
			overflowQueue.pop_front();

			if (!roundRobinIterator)
				advanceRoundRobin();

			iterateRoundRobin([&](const std::shared_ptr<InventoriedTileEntity> &inventoried, Direction direction) {
				inventoried->insertItem(stack, direction, &stack);
				return !stack;
			});

			// If the stack couldn't be fully inserted, put the remainder at the end of the overflow queue.
			// TODO!: copy here?
			if (stack)
				overflowQueue.push_back(std::move(stack));

			return;
		}

		// Doing this instead of std::erase_if so that perhaps later I could do something special on each removal.
		std::vector<PairSet::iterator> to_erase;
		auto extractions_lock = extractions.uniqueLock();

		for (auto iter = extractions.begin(), end = extractions.end(); iter != end; ++iter) {
			const auto &[position, direction] = *iter;

			auto inventoried = std::dynamic_pointer_cast<InventoriedTileEntity>(realm->tileEntityAt(position));
			if (!inventoried) {
				to_erase.push_back(iter);
				continue;
			}

			const InventoryPtr inventory = inventoried->getInventory(0);

			if (!inventory) {
				WARN("{} has no inventory 0.", inventoried->getName());
				return;
			}

			{
				auto inventory_lock = inventory->sharedLock();
				if (inventoried->empty())
					continue;
			}

			if (!roundRobinIterator)
				advanceRoundRobin();

			bool failed = false;
			auto inventory_lock = inventory->uniqueLock();

			std::shared_ptr<ItemFilter> extraction_filter;
			if (const auto pipe = std::dynamic_pointer_cast<Pipe>(realm->tileEntityAt(position + direction)))
				extraction_filter = pipe->itemFilters[flipDirection(direction)];

			inventoried->iterateExtractableItems(direction, [&, direction = direction](const ItemStackPtr &stack, Slot slot) {
				if (extraction_filter && !extraction_filter->isAllowed(stack, *inventory))
					return false;

				// It's possible we'll extract an item and put it right back.
				// If that happens, we don't want to notify the owner and potentially queue a broadcast.
				auto suppressor = inventory->suppress();

				ItemStackPtr extracted = inventoried->extractItem(direction, true, slot, -2);

				// This would be a little strange.
				if (!extracted) {
					WARN("Couldn't extract item indicated to be extractable from slot {}.", slot);
					return false;
				}

				const ItemCount original_count = extracted->count;

				// Try to insert the extracted item into insertion points until we either finish inserting all of it
				// or we run out of insertion points.
				iterateRoundRobin([&](const std::shared_ptr<InventoriedTileEntity> &round_robin, Direction round_robin_direction) -> bool {
					InventoryPtr round_robin_inventory = round_robin->getInventory(0);

					if (!round_robin_inventory)
						return false;

					if (const auto pipe = std::dynamic_pointer_cast<Pipe>(realm->tileEntityAt(round_robin->getPosition() + round_robin_direction))) {
						auto round_robin_inventory_lock = round_robin_inventory->sharedLock();
						if (std::shared_ptr<ItemFilter> insertion_filter = pipe->itemFilters[flipDirection(round_robin_direction)]; insertion_filter && !insertion_filter->isAllowed(extracted, *round_robin_inventory)) {
							return false;
						}
					}

					// TODO?: support multiple inventories in item networks
					auto lock = round_robin_inventory->uniqueLock();
					round_robin->insertItem(extracted, round_robin_direction, &extracted);
					return !extracted;
				}, inventoried);

				if (extracted) {
					const bool changed = extracted->count != original_count;

					// If there's anything left over, try putting it back into the inventory it was extracted from.
					if (ItemStackPtr new_leftover = inventory->add(extracted, slot)) {
						// If there's still anything left over, move it to the overflowQueue so we can try to insert it somewhere another time.
						// Theoretically this should never happen because we've locked the source inventory.
						// Also, because the source inventory changed, we need to cancel the suppressor.
						WARN("Can't put leftovers back into source inventory of type {}.", DEMANGLE(*inventory));
						suppressor.cancel(true);
						overflowQueue.push_back(std::move(new_leftover)); // TODO!: copy?
						// Because we're in a weird situation here, it might be safer to just cancel the iteration now.
						failed = true;
						return true;
					}

					// If we inserted it back but the size was different (due to partial insertion), we need to update the inventory.
					if (changed) {
						suppressor.cancel(true);
					}
				} else {
					// Because no leftovers means the extraction was successful and the source inventory changed, we need to undo the effect of the suppressor.
					suppressor.cancel(true);
				}

				return false;
			});

			if (failed)
				return;
		}

		for (const auto &iter: to_erase)
			extractions.erase(iter);
	}

	void ItemNetwork::lastPipeRemoved(Position where) {
		if (overflowQueue.empty())
			return;

		RealmPtr realm = weakRealm.lock();
		if (!realm) {
			WARN_("Item network destroyed while unable to drop overflow");
			return;
		}

		for (const ItemStackPtr &stack: overflowQueue)
			stack->spawn(Place{where, realm});

		// Just in case.
		overflowQueue.clear();
	}

	bool ItemNetwork::canWorkWith(const std::shared_ptr<TileEntity> &tile_entity) const {
		return std::dynamic_pointer_cast<InventoriedTileEntity>(tile_entity) != nullptr;
	}

	void ItemNetwork::reset() {
		roundRobinIterator = std::nullopt;
	}

	void ItemNetwork::advanceRoundRobin() {
		RealmPtr realm = weakRealm.lock();
		auto insertions_lock = insertions.sharedLock();

		if (!realm) {
			roundRobinIterator.getBase() = insertions.end();
			return;
		}

		if (insertions.size() == 1) {
			TileEntityPtr tile_entity = realm->tileEntityAt(insertions.begin()->first);

			if (std::dynamic_pointer_cast<HasInventory>(tile_entity))
				roundRobinIterator.getBase() = insertions.begin();
			else
				roundRobinIterator.getBase() = insertions.end();

			return;
		}

		if (!roundRobinIterator || *roundRobinIterator == insertions.end())
			roundRobinIterator.getBase() = insertions.begin();

		auto old_iter = *roundRobinIterator;
		bool has_inventory = false;

		// Keep searching for the next insertion that has an inventory until we reach the initial iterator.
		do {
			if (++*roundRobinIterator == insertions.end())
				roundRobinIterator.getBase() = insertions.begin();

			assert(!insertions.empty());
			assert(roundRobinIterator);

			auto iter = *roundRobinIterator;
			assert(iter != insertions.end());
			auto pair = *iter;
			auto pos = pair.first;

			has_inventory = std::dynamic_pointer_cast<HasInventory>(realm->tileEntityAt(pos)) != nullptr;
		} while (*roundRobinIterator != old_iter && !has_inventory);

		// If we haven't found an inventoried tile entity by this point, it means none exists among the insertion set;
		// therefore, we need to invalidate the iterator.
		if (!has_inventory)
			roundRobinIterator.getBase() = insertions.end();
	}

	std::pair<std::shared_ptr<InventoriedTileEntity>, Direction> ItemNetwork::getRoundRobin() {
		if (!roundRobinIterator)
			advanceRoundRobin();

		{
			auto lock = insertions.sharedLock();
			if (*roundRobinIterator == insertions.end())
				return {nullptr, Direction::Invalid};
		}

		RealmPtr realm = weakRealm.lock();
		if (!realm)
			return {nullptr, Direction::Invalid};

		const auto [position, direction] = **roundRobinIterator;

		TileEntityPtr tile_entity = realm->tileEntityAt(position);
		if (!tile_entity)
			return {nullptr, Direction::Invalid};

		return {std::dynamic_pointer_cast<InventoriedTileEntity>(tile_entity), direction};
	}

	void ItemNetwork::iterateRoundRobin(const std::function<bool(const std::shared_ptr<InventoriedTileEntity> &, Direction)> &function, const std::shared_ptr<TileEntity> &avoid) {
		auto old_iter = roundRobinIterator;
		size_t counter = 0;
		size_t max = 0;

		{
			auto lock = insertions.sharedLock();
			max = insertions.size();
		}

		do {
			advanceRoundRobin();
			if (const auto [tile_entity, direction] = getRoundRobin(); tile_entity) {
				if (tile_entity != avoid && function(tile_entity, direction))
					return;
			}
		} while (old_iter != roundRobinIterator.getBase() && ++counter < max);
	}
}
