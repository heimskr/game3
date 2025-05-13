#pragma once

#include "types/Layer.h"
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
			bool valid = false;

			ThreadContext():
				rng(std::chrono::system_clock::now().time_since_epoch().count()),
				threadID(std::this_thread::get_id()),
				valid(false) {}

			ThreadContext(uint_fast32_t seed, Index row_min, Index row_max, Index col_min, Index col_max):
				rng(seed),
				threadID(std::this_thread::get_id()),
				rowMin(row_min),
				rowMax(row_max),
				colMin(col_min),
				colMax(col_max),
				valid(true) {}

			/** [min, max] */
			template <std::integral T>
			T random(T min, T max) {
				return std::uniform_int_distribution(min, max)(rng);
			}

			/** [min, max) */
			template <std::floating_point T>
			T random(T min, T max) {
				return std::uniform_real_distribution(min, max)(rng);
			}

			/** variance >= 1 */
			template <typename T>
			T getPitch(T variance) {
				if (variance == 1) {
					return 1;
				}

				return std::uniform_real_distribution<T>(static_cast<T>(1.0) / variance, variance)(rng);
			}

			void rename(const char *);
			void rename(const std::string &);
	};

	extern thread_local ThreadContext threadContext;
}
