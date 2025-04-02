#include "util/Log.h"
#include "game/EnergyContainer.h"
#include "pipes/EnergyNetwork.h"
#include "realm/Realm.h"
#include "tileentity/EnergeticTileEntity.h"

namespace Game3 {
	EnergyNetwork::EnergyNetwork(size_t id_, const std::shared_ptr<Realm> &realm):
		PipeNetwork(id_, realm), HasEnergy(CAPACITY, 0) {}

	void EnergyNetwork::tick(const std::shared_ptr<Game> &game, Tick tick_id) {
		if (!canTick(tick_id)) {
			PipeNetwork::tick(game, tick_id);
			return;
		}

		PipeNetwork::tick(game, tick_id);

		auto this_lock = uniqueLock();

		RealmPtr realm = weakRealm.lock();
		if (!realm || insertions.empty())
			return;

		EnergyAmount &energy = energyContainer->energy;
		auto energy_lock = energyContainer->uniqueLock();

		if (0 < energy) {
			energy = distribute(energy);
			// if (0 < energy)
			// 	return;
		}

		const EnergyAmount capacity = getEnergyCapacity();
		assert(energy <= capacity);

		{
			std::vector<PairSet::iterator> to_erase;
			auto extractions_lock = extractions.uniqueLock();

			for (auto iter = extractions.begin(); iter != extractions.end(); ++iter) {
				const auto [position, direction] = *iter;
				auto energetic = std::dynamic_pointer_cast<EnergeticTileEntity>(realm->tileEntityAt(position));
				if (!energetic) {
					to_erase.push_back(iter);
					continue;
				}

				energy += energetic->extractEnergy(direction, true, capacity - energy);
				if (capacity <= energy)
					energy = distribute(energy);
			}

			for (const auto &iter: to_erase)
				extractions.erase(iter);
		}

		energy = distribute(energy);
	}

	bool EnergyNetwork::canWorkWith(const std::shared_ptr<TileEntity> &tile_entity) const {
		return std::dynamic_pointer_cast<EnergeticTileEntity>(tile_entity) != nullptr;
	}

	EnergyAmount EnergyNetwork::distribute(EnergyAmount amount) {
		if (amount == 0)
			return 0;

		RealmPtr realm = weakRealm.lock();
		if (!realm)
			return amount;

		std::vector<std::pair<std::shared_ptr<EnergeticTileEntity>, Direction>> accepting_insertions;

		{
			auto insertions_lock = insertions.uniqueLock();
			accepting_insertions.reserve(insertions.size());

			std::erase_if(insertions, [&](const std::pair<Position, Direction> &pair) {
				const auto [position, direction] = pair;
				auto energetic = std::dynamic_pointer_cast<EnergeticTileEntity>(realm->tileEntityAt(position));
				if (!energetic)
					return true;

				if (energetic->canInsertEnergy(1, direction))
					accepting_insertions.emplace_back(energetic, direction);

				return false;
			});
		}

		if (accepting_insertions.empty())
			return amount;

		size_t insertions_remaining = accepting_insertions.size();

		for (const auto &[insertion, direction]: accepting_insertions) {
			const EnergyAmount to_distribute = amount / insertions_remaining;
			const EnergyAmount leftover = insertion->addEnergy(to_distribute, direction);
			const EnergyAmount distributed = to_distribute - leftover;
			amount -= distributed;
			--insertions_remaining;
		}

		return amount;
	}
}
