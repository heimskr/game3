#pragma once

#include "Types.h"
#include "game/Container.h"
#include "threading/HasMutex.h"

namespace Game3 {
	class EnergyContainer: public Container, public HasMutex<> {
		public:
			// I wish I could just use std::atomic for this.
			EnergyAmount energy;
			EnergyAmount capacity;

			EnergyContainer(EnergyAmount capacity_, EnergyAmount energy_ = 0):
				energy(energy_), capacity(capacity_) {}

			/** Doesn't lock the mutex. */
			bool canInsert(EnergyAmount);

			/** Returns the amount of energy left over. Doesn't lock the mutex. */
			EnergyAmount add(EnergyAmount);

			/** Doesn't lock the mutex. */
			bool remove(EnergyAmount);
	};
}
