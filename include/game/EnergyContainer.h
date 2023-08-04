#pragma once

#include "Types.h"
#include "game/Container.h"

#include <mutex>
#include <shared_mutex>

namespace Game3 {
	class EnergyContainer: public Container {
		public:
			// I wish I could just use std::atomic for this.
			EnergyAmount energy;
			EnergyAmount capacity;
			std::shared_mutex mutex;

			auto uniqueLock() { return std::unique_lock{mutex}; }
			auto sharedLock() { return std::shared_lock{mutex}; }

			EnergyContainer(EnergyAmount capacity_, EnergyAmount energy_ = 0):
				energy(energy_), capacity(capacity_) {}

			/** Doesn't lock the mutex. */
			bool canInsert(EnergyAmount);

			/** Returns the amount of energy left over. Doesn't lock the mutex. */
			EnergyAmount add(EnergyAmount);

			bool remove(EnergyAmount);
	};
}
