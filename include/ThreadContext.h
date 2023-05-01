#pragma once

#include <chrono>
#include <memory>
#include <random>
#include <thread>

#include "Types.h"

namespace Game3 {
	class Game;

	class ThreadContext {
		public:
			std::default_random_engine rng;
			std::thread::id threadID;
			Index rowMin = -1;
			Index rowMax = -1;
			Index colMin = -1;
			Index colMax = -1;
			size_t updateNeighborsDepth = 0;
			bool layer2Updated = false;
			bool valid = false;

			ThreadContext():
				rng(std::chrono::system_clock::now().time_since_epoch().count()),
				threadID(std::this_thread::get_id()),
				valid(false) {}

			ThreadContext(const std::shared_ptr<Game> &game_, uint_fast32_t seed, Index row_min, Index row_max, Index col_min, Index col_max):
				rng(seed),
				threadID(std::this_thread::get_id()),
				rowMin(row_min),
				rowMax(row_max),
				colMin(col_min),
				colMax(col_max),
				valid(true),
				game(game_) {}

			/** [min, max] */
			int64_t random(int64_t min, int64_t max) {
				return std::uniform_int_distribution(min, max)(rng);
			}

			Game & getGame() {
				if (auto locked = game.lock())
					return *locked;
				throw std::runtime_error("Game is null in ThreadContext");
			}

		private:
			std::weak_ptr<Game> game;
	};

	extern thread_local ThreadContext threadContext;
}
