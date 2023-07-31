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

		auto overflow_lock = overflowQueue.uniqueLock();
		auto round_robin_lock = roundRobinIterator.uniqueLock();

		// Every so often, if there's anything in the overflowQueue, we try to insert that somewhere instead of extracting anything more.
		if (overflowPeriod != 0 && tick_id % overflowPeriod == 0 && !overflowQueue.empty()) {
			std::optional<ItemStack> stack = std::move(overflowQueue.front());
			overflowQueue.pop_front();

			if (!roundRobinIterator)
				advanceRoundRobin();

			iterateRoundRobin([&](const std::shared_ptr<InventoriedTileEntity> &inventoried, Direction direction) {
				inventoried->insertItem(*stack, direction, &stack);
				return !stack.has_value();
			});

			// If the stack couldn't be fully inserted, put the remainder at the end of the overflow queue.
			if (stack)
				overflowQueue.push_back(std::move(*stack));

			return;
		}

		// Doing this instead of std::erase_if so that perhaps later I could do something special on each removal.
		std::vector<PairSet::iterator> to_erase;

		for (auto iter = extractions.begin(), end = extractions.end(); iter != end; ++iter) {
			const auto &[position, direction] = *iter;

			auto inventoried = std::dynamic_pointer_cast<InventoriedTileEntity>(realm->tileEntityAt(position));
			if (!inventoried) {
				to_erase.push_back(iter);
				continue;
			}

			{
				auto inventory_lock = inventoried->inventory->sharedLock();
				if (inventoried->empty())
					continue;
			}

			if (!roundRobinIterator)
				advanceRoundRobin();

			bool failed = false;
			auto inventory_lock = inventoried->inventory->uniqueLock();

			inventoried->iterateExtractableItems(direction, [&](const ItemStack &, Slot slot) {
				// It's possible we'll extract an item and put it right back.
				// If that happens, we don't want to notify the owner and potentially queue a broadcast.
				auto suppressor = inventoried->inventory->suppress();

				std::optional<ItemStack> extracted = inventoried->extractItem(direction, true, slot);

				// This would be a little strange.
				if (!extracted) {
					WARN("Couldn't extract item indicated to be extractable.");
					return false;
				}

				const ItemCount original_count = extracted->count;

				// Try to insert the extracted item into insertion points until we either finish inserting all of it
				// or we run out of insertion points.
				iterateRoundRobin([&](const std::shared_ptr<InventoriedTileEntity> &round_robin, Direction round_robin_direction) -> bool {
					auto lock = round_robin->inventory->uniqueLock();
					round_robin->insertItem(*extracted, round_robin_direction, &extracted);
					return !extracted.has_value();
				}, inventoried);

				if (extracted) {
					const bool changed = extracted->count != original_count;

					// If there's anything left over, try putting it back into the inventory it was extracted from.
					if (std::optional<ItemStack> new_leftover = inventoried->inventory->add(*extracted)) {
						// If there's still anything left over, move it to the overflowQueue so we can try to insert it somewhere another time.
						// Theoretically this should never happen because we've locked the source inventory.
						// Also, because the source inventory changed, we need to cancel the suppressor.
						WARN("Can't put leftovers back into source inventory.");
						suppressor.cancel(true);
						overflowQueue.push_back(std::move(*extracted));
						// Since we're in a weird situation here, it might be safer to just cancel the iteration here.
						failed = true;
						return true;
					}

					// If we inserted it back but the size was different (due to partial insertion), we need to update the inventory.
					if (changed)
						suppressor.cancel(true);
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
			has_inventory = std::dynamic_pointer_cast<HasInventory>(realm->tileEntityAt((*roundRobinIterator)->first)) != nullptr;
		} while (*roundRobinIterator != old_iter && !has_inventory);

		// If we haven't found an inventoried tile entity by this point, it means none exists among the insertion set;
		// therefore, we need to invalidate the iterator.
		if (!has_inventory)
			roundRobinIterator.getBase() = insertions.end();
	}

	std::pair<std::shared_ptr<InventoriedTileEntity>, Direction> ItemNetwork::getRoundRobin() {
		if (!roundRobinIterator)
			advanceRoundRobin();

		if (*roundRobinIterator == insertions.end())
			return {nullptr, Direction::Invalid};

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
		do {
			advanceRoundRobin();
			if (const auto [tile_entity, direction] = getRoundRobin(); tile_entity) {
				if (tile_entity != avoid && function(tile_entity, direction))
					return;
			}
		} while (old_iter != roundRobinIterator.getBase());
	}
}
