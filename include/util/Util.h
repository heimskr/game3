#pragma once

#include <random>
#include <vector>

namespace Game3 {
	template <typename T, typename R = std::default_random_engine>
	void shuffle(T &container, typename R::result_type seed = 0) {
		R rng;
		if (seed == 0)
			rng.seed(time(nullptr));
		else
			rng.seed(seed);
		std::shuffle(container.begin(), container.end(), rng);
	}

	template <typename T, typename R = std::default_random_engine>
	T::value_type & choose(T &container, typename R::result_type seed = 0) {
		if (container.empty())
			throw std::invalid_argument("Container is empty");
		R rng;
		if (seed == 0)
			rng.seed(time(nullptr));
		else
			rng.seed(seed);
		return container.at(rng() % container.size());
	}
}