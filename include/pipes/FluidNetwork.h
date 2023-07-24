#pragma once

#include "pipes/PipeNetwork.h"

namespace Game3 {
	class Inventory;

	class FluidNetwork: public PipeNetwork {
		public:
			using PipeNetwork::PipeNetwork;

			PipeType getType() const final { return PipeType::Fluid; }

			void tick(Tick) final;
	};
}
