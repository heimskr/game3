#include "game/EnergyContainer.h"
#include "game/HasEnergy.h"
#include "net/Buffer.h"

#include <mutex>

namespace Game3 {
	HasEnergy::HasEnergy(std::shared_ptr<EnergyContainer> container):
		energyContainer(std::move(container)) {}

	HasEnergy::HasEnergy(EnergyAmount amount):
		energyContainer(std::make_shared<EnergyContainer>(amount)) {}

	EnergyAmount HasEnergy::addEnergy(EnergyAmount to_add) {
		assert(energyContainer);
		EnergyAmount remainder{};
		{
			std::unique_lock lock{energyContainer->mutex};
			remainder = energyContainer->add(to_add);
		}
		if (remainder != to_add)
			energyUpdated();
		return remainder;
	}

	EnergyAmount HasEnergy::getEnergyCapacity() {
		assert(energyContainer);
		std::shared_lock lock{energyContainer->mutex};
		return energyContainer->capacity;
	}

	EnergyAmount HasEnergy::getEnergy() {
		assert(energyContainer);
		// Probably don't need the lock, but TSAN might complain.
		std::shared_lock lock{energyContainer->mutex};
		return energyContainer->energy;
	}

	void HasEnergy::setEnergy(EnergyAmount new_amount) {
		{
			std::unique_lock lock{energyContainer->mutex};
			energyContainer->energy = new_amount;
		}
		energyUpdated();
	}

	void HasEnergy::encode(Buffer &buffer) {
		std::shared_lock lock{energyContainer->mutex};
		buffer << energyContainer->energy;
	}

	void HasEnergy::decode(Buffer &buffer) {
		std::unique_lock lock{energyContainer->mutex};
		buffer >> energyContainer->energy;
	}
}
