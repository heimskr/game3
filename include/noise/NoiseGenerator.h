#pragma once

#include <random>

#include "SquareVector.h"

namespace Game3 {
	template <typename T, typename R = std::mt19937_64>
	class NoiseGenerator {
		public:
			R rng;
			R::result_type seed;
			SquareVector<T> output;

			NoiseGenerator(R::result_type seed_): rng(seed_), seed(seed_), output(0, 0) {}

			virtual ~NoiseGenerator() = default;

			virtual decltype(output) & run() = 0;
			virtual decltype(output) & run(T origin_x, T origin_y) = 0;
			virtual void normalize(T low, T high) = 0;
	};
}
