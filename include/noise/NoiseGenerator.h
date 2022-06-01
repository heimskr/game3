#pragma once

#include <random>

#include "SquareVector.h"

namespace Game3 {
	template <typename T, typename R = std::mt19937_64>
	class NoiseGenerator {
		public:
			R rng;
			R::result_type seed;
			size_t sideLength;
			SquareVector<T> output;

			NoiseGenerator(R::result_type seed_, size_t side_length): rng(seed_), seed(seed_), sideLength(side_length), output(side_length, side_length) {}

			virtual ~NoiseGenerator() = default;

			virtual decltype(output) & run() = 0;
			virtual decltype(output) & run(T origin_x, T origin_y) = 0;
			virtual void normalize(T low, T high) = 0;
	};
}
