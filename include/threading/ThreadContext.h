#pragma once

#include "Layer.h"
#include "types/Types.h"

#include <chrono>
#include <memory>
#include <random>
#include <thread>
#include <unordered_set>

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
			std::unordered_set<Layer> updatedLayers;
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

			template <std::integral T>
			/** [min, max] */
			T random(T min, T max) {
				return std::uniform_int_distribution(min, max)(rng);
			}

			template <std::floating_point T>
			/** [min, max) */
			T random(T min, T max) {
				return std::uniform_real_distribution(min, max)(rng);
			}

			template <typename T>
			T getPitch(T variance) {
				return std::uniform_real_distribution<T>(1.0 / variance, variance)(rng);
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
