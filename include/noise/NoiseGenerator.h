#pragma once

#include <random>

#include "SquareVector.h"

namespace Game3 {
	template <typename T, typename R = std::mt19937_64>
	class NoiseGenerator {
		public:
			SquareVector<T> output;
			R random;

			NoiseGenerator() = default;
			virtual ~NoiseGenerator() = 0;

			virtual void run() = 0;
			virtual void normalize(T low = 0., T high = 1.) = 0;
	};
}
