#include "game/HasEnergy.h"
#include "net/Buffer.h"

#include <mutex>

namespace Game3 {
	EnergyAmount HasEnergy::addEnergy(EnergyAmount to_add) {
		const EnergyAmount max = getEnergyCapacity();

		std::unique_lock lock{energyMutex};

		// Handle integer overflow
		if (energyAmount + to_add < energyAmount) {
			const EnergyAmount remainder = to_add - (std::numeric_limits<EnergyAmount>::max() - energyAmount);
			energyAmount = max;
			lock.unlock();
			energyUpdated();
			return remainder;
		}

		if (max < energyAmount + to_add) {
			const EnergyAmount remainder = energyAmount + to_add - max;
			energyAmount = max;
			lock.unlock();
			energyUpdated();
			return remainder;
		}

		energyAmount += to_add;
		lock.unlock();
		energyUpdated();
		return 0;
	}

	void HasEnergy::encode(Buffer &buffer) {
		std::shared_lock lock{energyMutex};
		buffer << energyAmount;
	}

	void HasEnergy::decode(Buffer &buffer) {
		std::unique_lock lock{energyMutex};
		buffer >> energyAmount;
	}
}
