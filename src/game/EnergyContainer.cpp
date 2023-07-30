#include "Log.h"
#include "game/EnergyContainer.h"

namespace Game3 {
	bool EnergyContainer::canInsert(EnergyAmount to_add) {
		const EnergyAmount sum = energy + to_add;
		return energy <= sum && sum <= capacity;
	}

	EnergyAmount EnergyContainer::add(EnergyAmount to_add) {
		// Handle integer overflow
		if (energy + to_add < energy) {
			const EnergyAmount remainder = to_add - (capacity - energy);
			energy = capacity;
			return remainder;
		}

		if (capacity < energy + to_add) {
			const EnergyAmount remainder = energy + to_add - capacity;
			energy = capacity;
			return remainder;
		}

		energy += to_add;
		return 0;
	}
}