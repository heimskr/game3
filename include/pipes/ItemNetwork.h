#pragma once

#include "item/Item.h"
#include "pipes/PipeNetwork.h"

#include <deque>

namespace Game3 {
	class InventoriedTileEntity;

	class ItemNetwork: public PipeNetwork {
		public:
			Tick overflowPeriod = 10;

			using PipeNetwork::PipeNetwork;

			PipeType getType() const final { return PipeType::Item; }

			void tick(Tick) final;
			void lastPipeRemoved(Position) final;
			bool canWorkWith(const std::shared_ptr<TileEntity> &) const final;

			inline size_t overflowCount() const { return overflowQueue.size(); }

		private:
			std::optional<PairSet::iterator> roundRobinIterator;
			Lockable<std::deque<ItemStack>> overflowQueue;

			void advanceRoundRobin();
			std::pair<std::shared_ptr<InventoriedTileEntity>, Direction> getRoundRobin();
	};
}
