#pragma once

#include "types/Types.h"

#include <unordered_set>

namespace Game3 {
	class Game;

	class Tickable {
		public:
			Tickable() = default;

			virtual ~Tickable() = default;

			virtual Game & getGame() = 0;
			virtual Tick enqueueTick() = 0;

			void tryEnqueueTick();
			void didTick();
			Tick tickEnqueued(Tick);

		private:
			std::unordered_set<Tick> tickSet;
	};
}
