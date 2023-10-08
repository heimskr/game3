#pragma once

#include "item/Item.h"
#include "pipes/PipeNetwork.h"

#include <deque>
#include <functional>

namespace Game3 {
	class InventoriedTileEntity;
	class TileEntity;

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
			Lockable<std::optional<PairSet::iterator>> roundRobinIterator;
			Lockable<std::deque<ItemStack>> overflowQueue;

			/** Doesn't lock anything. */
			void advanceRoundRobin();
			/** Doesn't lock anything. */
			std::pair<std::shared_ptr<InventoriedTileEntity>, Direction> getRoundRobin();
			/** Doesn't lock anything.
			 *  Iteration stops once the function returns true or a full loop of all insertions has happened. */
			void iterateRoundRobin(const std::function<bool(const std::shared_ptr<InventoriedTileEntity> &, Direction)> &, const std::shared_ptr<TileEntity> &avoid = nullptr);
	};
}
