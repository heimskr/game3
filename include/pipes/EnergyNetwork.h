#pragma once

#include "pipes/PipeNetwork.h"

namespace Game3 {
	class Inventory;

	class EnergyNetwork: public PipeNetwork {
		public:
			using PipeNetwork::PipeNetwork;

			PipeType getType() const final { return PipeType::Energy; }

			void tick(Tick) final;
	};
}
