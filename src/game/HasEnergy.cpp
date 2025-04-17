#include "game/EnergyContainer.h"
#include "game/HasEnergy.h"
#include "net/Buffer.h"

namespace Game3 {
	HasEnergy::HasEnergy(std::shared_ptr<EnergyContainer> container):
		energyContainer(std::move(container)) {}

	HasEnergy::HasEnergy(EnergyAmount capacity, EnergyAmount amount):
		energyContainer(std::make_shared<EnergyContainer>(capacity, amount)) {}

	EnergyAmount HasEnergy::addEnergy(EnergyAmount to_add) {
		assert(energyContainer);
		const EnergyAmount remainder = energyContainer->add(to_add);
		if (remainder != to_add) {
			energyUpdated();
		}
		return remainder;
	}

	EnergyAmount HasEnergy::getEnergyCapacity() {
		assert(energyContainer);
		return energyContainer->capacity;
	}

	EnergyAmount HasEnergy::getEnergy() {
		assert(energyContainer);
		return energyContainer->energy;
	}

	void HasEnergy::setEnergy(EnergyAmount new_amount) {
		energyContainer->energy = new_amount;
		energyUpdated();
	}

	void HasEnergy::encode(Buffer &buffer) {
		buffer << energyContainer->energy;
	}

	void HasEnergy::decode(Buffer &buffer) {
		buffer >> energyContainer->energy;
	}
}
