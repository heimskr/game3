#pragma once

#include "types/Types.h"

#include <atomic>
#include <shared_mutex>

namespace Game3 {
	class Buffer;
	class EnergyContainer;

	/** None of these methods should (or do) lock anything. */
	class HasEnergy {
		public:
			std::shared_ptr<EnergyContainer> energyContainer;

			HasEnergy() = default;
			explicit HasEnergy(std::shared_ptr<EnergyContainer>);
			HasEnergy(EnergyAmount capacity, EnergyAmount amount);

			virtual ~HasEnergy() = default;

			/** Returns the amount of energy unable to be added. */
			virtual EnergyAmount addEnergy(EnergyAmount);
			virtual EnergyAmount getEnergyCapacity();
			virtual EnergyAmount getEnergy();
			virtual void setEnergy(EnergyAmount);

			virtual void energyUpdated() {}

			void encode(Buffer &);
			void decode(Buffer &);
	};
}
