#pragma once

#include "game/HasEnergy.h"
#include "pipes/PipeNetwork.h"

namespace Game3 {
	class EnergyNetwork: public PipeNetwork, public HasEnergy {
		public:
			constexpr static EnergyAmount CAPACITY = 10'000;

			EnergyNetwork(size_t id_, const std::shared_ptr<Realm> &);

			Substance getType() const final { return Substance::Energy; }

			void tick(const std::shared_ptr<Game> &, Tick) final;
			bool canWorkWith(const std::shared_ptr<TileEntity> &) const final;

			EnergyAmount distribute(EnergyAmount);
	};
}
