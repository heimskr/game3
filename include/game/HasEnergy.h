#pragma once

#include "Types.h"

#include <atomic>
#include <shared_mutex>

namespace Game3 {
	class Buffer;

	class HasEnergy {
		public:
			HasEnergy() = default;

			explicit HasEnergy(EnergyAmount energy_amount):
				energyAmount(energy_amount) {}

			/** Returns the amount of energy unable to be added. */
			virtual EnergyAmount addEnergy(EnergyAmount);
			virtual EnergyAmount getEnergyCapacity() = 0;
			virtual EnergyAmount getEnergy();
			virtual void setEnergy(EnergyAmount);

			virtual void energyUpdated() {}

			void encode(Buffer &);
			void decode(Buffer &);

		protected:
			// I wish I could just use std::atomic for this.
			EnergyAmount energyAmount = 0;
			std::shared_mutex energyMutex;
	};
}
