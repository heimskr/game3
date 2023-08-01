#pragma once

#include "game/HasEnergy.h"
#include "pipes/PipeNetwork.h"

namespace Game3 {
	class Inventory;

	class EnergyNetwork: public PipeNetwork, public HasEnergy {
		public:
			constexpr static EnergyAmount CAPACITY = 100'000;

			EnergyNetwork(size_t id_, const std::shared_ptr<Realm> &);

			PipeType getType() const final { return PipeType::Energy; }

			void tick(Tick) final;
			bool canWorkWith(const std::shared_ptr<TileEntity> &) const final;

			EnergyAmount distribute(EnergyAmount);
	};
}
