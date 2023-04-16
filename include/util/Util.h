#pragma once

#include <algorithm>
#include <chrono>
#include <csignal>
#include <iostream>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>

#define TRY try {
#define CATCH } catch (std::exception &err) { std::cerr << "\e[31m" << __FILE__ << ':' << __LINE__ << ": " << err.what() << "\e[39m\n"; }
#define PRINTMAT4(m) do { std::cerr << "[[[\n"; for (int r = 0; r < 4; ++r) { for (int c = 0; c < 4; ++c) std::cerr << (m)[c][r] << ' '; std::cerr << '\n'; } std::cerr << "]]]\n"; } while (0)

namespace Game3 {
	extern std::default_random_engine utilRNG;

	long parseLong(const std::string &, int base = 10);
	long parseLong(const char *, int base = 10);
	long parseLong(std::string_view, int base = 10);
	unsigned long parseUlong(const std::string &, int base = 10);
	unsigned long parseUlong(const char *, int base = 10);
	unsigned long parseUlong(std::string_view, int base = 10);

	inline double fractional(double d) {
		return std::modf(d, &d);
	}

	inline std::chrono::system_clock::time_point getTime() {
		return std::chrono::system_clock::now();
	}

	template <typename T = std::chrono::nanoseconds>
	inline T timeDifference(std::chrono::system_clock::time_point old_time) {
		return std::chrono::duration_cast<T>(getTime() - old_time);
	}

	static inline std::default_random_engine::result_type getRandom(std::default_random_engine::result_type seed = 0) {
		if (seed == 0)
			return utilRNG();
		std::default_random_engine rng;
		rng.seed(seed);
		return rng();
	}

	template <typename T, typename R = std::default_random_engine>
	void shuffle(T &container, typename R::result_type seed = 0) {
		if (seed == 0) {
			std::shuffle(container.begin(), container.end(), utilRNG);
		} else {
			R rng;
			rng.seed(seed);
			std::shuffle(container.begin(), container.end(), rng);
		}
	}

	template <typename T>
	typename T::value_type & choose(T &container, typename std::default_random_engine::result_type seed = 0) {
		if (container.empty())
			throw std::invalid_argument("Container is empty");
		return container.at(getRandom(seed) % container.size());
	}

	template <typename T>
	const typename T::value_type & choose(const T &container, typename std::default_random_engine::result_type seed = 0) {
		if (container.empty())
			throw std::invalid_argument("Container is empty");
		return container.at(getRandom(seed) % container.size());
	}

	template <typename T, typename R>
	typename T::value_type & choose(T &container, R &rng) {
		if (container.empty())
			throw std::invalid_argument("Container is empty");
		return container.at(rng() % container.size());
	}

	template <typename T, typename R>
	const typename T::value_type & choose(const T &container, R &rng) {
		if (container.empty())
			throw std::invalid_argument("Container is empty");
		return container.at(rng() % container.size());
	}

	template <typename T, typename R>
	const T & choose(const std::set<T> &set, R &rng) {
		if (set.empty())
			throw std::invalid_argument("Set is empty");
		return *std::next(set.begin(), std::uniform_int_distribution(static_cast<size_t>(0), set.size() - 1)(rng));
	}
}