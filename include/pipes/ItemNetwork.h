#pragma once

#include "pipes/PipeNetwork.h"

namespace Game3 {
	class Inventory;

	class ItemNetwork: public PipeNetwork {
		public:
			using PipeNetwork::PipeNetwork;

			PipeType getType() const final { return PipeType::Item; }

			void tick(Tick) final;

		private:
			std::optional<PairSet::iterator> roundRobinIterator;
			std::shared_ptr<Inventory> cachedRoundRobinInventory;

			void advanceRoundRobin();
			std::shared_ptr<Inventory> getRoundRobinInventory();
	};
}
