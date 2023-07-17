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

			if (!roundRobinIterator)
				roundRobinIterator = insertions.begin();

			auto &rr_iter = *roundRobinIterator;

			if (rr_iter == insertions.end())
				continue;

			if (auto has_inventory = tile_entity->cast<HasInventory>(); has_inventory && has_inventory->inventory) {
				auto &inventory = *has_inventory->inventory;
				// inventory.
				if (!inventory.empty()) {
					auto &storage = inventory.getStorage();
					auto lock = storage.sharedLock();
					const auto &[slot, stack] = *storage.begin();

				}
			}
		}

		for (const auto &iter: to_erase) {
			extractions.erase(iter);
		}
	}
}
