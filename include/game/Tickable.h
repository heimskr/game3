#pragma once

#include "threading/Atomic.h"
#include "types/Types.h"

#include <unordered_set>

namespace Game3 {
	class Game;

	class Tickable {
		public:
			Tickable() = default;

			virtual ~Tickable() = default;

			virtual std::shared_ptr<Game> getGame() const = 0;
			virtual Tick enqueueTick() = 0;

			/** If the initial tick has occurred, this simply returns false.
			 *  Otherwise, it marks the initial tick as having occurred and returns true. */
			bool tryInitialTick();

			/** Enqueues a tick for the next tick ID if one has not already been enqueued. */
			void tryEnqueueTick();

			/** Removes the current tick ID from the tickset. */
			void didTick();

			/** Inserts a tick ID into the tickset. */
			Tick tickEnqueued(Tick);

		private:
			Atomic<bool> initialTickDone = false;
			std::unordered_set<Tick> tickSet;
	};
}
