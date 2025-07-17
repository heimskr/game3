#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <charconv>
#include <chrono>
#include <concepts>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "util/Concepts.h"
#include "util/Hex.h"
#include "util/Math.h"

namespace Game3 {
	extern std::default_random_engine utilRNG;

	template <typename... Ts>
	void zeroOut(Ts &...things) {
		((things = 0), ...);
	}

	template <typename T, typename... Us>
	bool isAny(T &&thing, Us &&...possibilities) {
		return (... || (std::forward<T>(thing) == std::forward<Us>(possibilities)));
	}

	/** Splits a string by a given delimiter. If condense is true, empty strings won't be included in the output. */
	template <typename T = std::string_view>
	std::vector<T> split(std::string_view str, std::string_view delimiter, bool condense = true);

	std::string_view trimLeft(std::string_view, std::string_view to_remove = " \t\r\n");
	std::string_view trimRight(std::string_view, std::string_view to_remove = " \t\r\n");
	std::string_view trim(std::string_view, std::string_view to_remove = " \t\r\n");

	template <typename C>
	std::string join(C &&container, std::string_view delimiter = " ") {
		std::stringstream ss;
		for (bool first = true; const auto &item: container) {
			if (first) {
				first = false;
			} else {
				ss << delimiter;
			}
			ss << item;
		}
		return std::move(ss).str();
	}

	template <typename S, typename T>
	void appendSpan(S &raw, const std::span<T> &source) {
		const size_t byte_count = source.size() * sizeof(source[0]);
		if constexpr (std::endian::native == std::endian::little) {
			const size_t start = raw.size();
			raw.resize(raw.size() + byte_count);
			std::memcpy(&raw[start], source.data(), byte_count);
		} else {
			raw.reserve(raw.size() + byte_count);
			for (const auto item: source)
				for (size_t i = 0; i < sizeof(item); ++i)
					raw.push_back((item >> (8 * i)) & 0xff);
		}
	}

	template <typename S, typename T>
	void appendBytes(S &raw, T item) {
		for (size_t i = 0; i < sizeof(item); ++i)
			raw.push_back((item >> (8 * i)) & 0xff);
	}

	template <typename C>
	std::string hexString(const C &container, bool spaces) {
		std::stringstream ss;
		ss.imbue(std::locale("C"));
		bool first = true;
		for (const auto item: container) {
			if (spaces) {
				if (first)
					first = false;
				else
					ss << ' ';
			}
			ss << std::hex << std::setw(2 * sizeof(item)) << std::setfill('0') << std::right;
			if constexpr (sizeof(item) == 1)
				ss << static_cast<uint16_t>(static_cast<std::make_unsigned_t<decltype(item)>>(item));
			else
				ss << static_cast<std::make_unsigned_t<decltype(item)>>(item);
		}
		return std::move(ss).str();
	}

	template <typename T>
	T unhex(std::string_view str) {
		T out;
		uint8_t buffer = 0;
		bool buffered = false;

		for (const char ch: str) {
			if (ch == ' ')
				continue;
			if (buffered) {
				out.emplace_back((buffer << 4) | fromHex(ch));
				buffered = false;
			} else {
				buffered = fromHex(ch);
				buffered = true;
			}
		}

		if (buffered)
			throw std::invalid_argument("Invalid hex string length");

		return out;
	}

	template <typename T, template <typename...> typename C, typename... Args>
	std::unordered_set<std::shared_ptr<T>> filterWeak(const C<std::weak_ptr<T>, Args...> &container) {
		std::unordered_set<std::shared_ptr<T>> out;
		for (const auto &weak: container)
			if (auto locked = weak.lock())
				out.insert(locked);
		return out;
	}

	long parseLong(const std::string &, int base = 10);
	long parseLong(const char *, int base = 10);
	long parseLong(std::string_view, int base = 10);
	unsigned long parseUlong(const std::string &, int base = 10);
	unsigned long parseUlong(const char *, int base = 10);
	unsigned long parseUlong(std::string_view, int base = 10);

	std::string escape(std::string_view);

	void openInBrowser(const std::string &url);

	template <std::integral I>
	I parseNumber(std::string_view view, int base = 10) {
		I out{};
		std::from_chars_result result = std::from_chars(view.begin(), view.end(), out, base);
		if (result.ec != std::errc{} || result.ptr - view.begin() != std::ssize(view)) {
			throw std::invalid_argument("Not an integer: \"" + std::string(view) + "\"");
		}
		return out;
	}

	template <std::floating_point F>
	F parseNumber(std::string_view view) {
#if defined(__APPLE__) && defined(__clang__)
		// Current Clang on macOS seems to hate from_chars for floating point types.
		std::string str(view);
		char *endptr = nullptr;
		double out = strtod(str.c_str(), &endptr);
		if (str.c_str() + str.size() != endptr) {
			throw std::invalid_argument("Not a floating point: \"" + str + "\"");
		}
		return out;
#else
		F out{};
		std::from_chars_result result = std::from_chars(view.begin(), view.end(), out);
		if (result.ec != std::errc{} || result.ptr - view.begin() != std::ssize(view)) {
			throw std::invalid_argument("Not a floating point: \"" + std::string(view) + "\"");
		}
		return out;
#endif
	}

	using SystemTimePoint = std::chrono::system_clock::time_point;

	inline SystemTimePoint getTime() {
		return std::chrono::system_clock::now();
	}

	template <typename T = std::chrono::nanoseconds>
	inline T timeDifference(SystemTimePoint old_time) {
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
	typename T::value_type & choose(T &container, R &&rng) {
		if (container.empty())
			throw std::invalid_argument("Container is empty");
		return container.at(std::uniform_int_distribution<size_t>(0, container.size() - 1)(rng));
	}

	// Here be ugly duplication

	template <typename T, typename R>
	T & choose(std::list<T> &container, R &&rng) {
		if (container.empty())
			throw std::invalid_argument("Container is empty");
		return *std::next(container.begin(), std::uniform_int_distribution(static_cast<size_t>(0), container.size() - 1)(rng));
	}

	template <typename T, typename R>
	T & choose(std::set<T> &set, R &&rng) {
		if (set.empty())
			throw std::invalid_argument("Set is empty");
		return *std::next(set.begin(), std::uniform_int_distribution(static_cast<size_t>(0), set.size() - 1)(rng));
	}

	template <typename T, typename R>
	T & choose(std::unordered_set<T> &set, R &&rng) {
		if (set.empty())
			throw std::invalid_argument("Set is empty");
		return *std::next(set.begin(), std::uniform_int_distribution(static_cast<size_t>(0), set.size() - 1)(rng));
	}

	template <typename T, typename R>
	const typename T::value_type & choose(const T &container, R &&rng) {
		if (container.empty())
			throw std::invalid_argument("Container is empty");
		return container.at(std::uniform_int_distribution(static_cast<size_t>(0), container.size() - 1)(rng));
	}

	template <typename T, typename R>
	const T & choose(const std::list<T> &container, R &&rng) {
		if (container.empty())
			throw std::invalid_argument("Container is empty");
		return *std::next(container.begin(), std::uniform_int_distribution(static_cast<size_t>(0), container.size() - 1)(rng));
	}

	template <typename T, typename R>
	const T & choose(const std::unordered_set<T> &set, R &&rng) {
		if (set.empty())
			throw std::invalid_argument("Set is empty");
		return *std::next(set.begin(), std::uniform_int_distribution(static_cast<size_t>(0), set.size() - 1)(rng));
	}

	template <typename T, typename R>
	const T & choose(const std::set<T> &set, R &&rng) {
		if (set.empty())
			throw std::invalid_argument("Set is empty");
		return *std::next(set.begin(), std::uniform_int_distribution(static_cast<size_t>(0), set.size() - 1)(rng));
	}

	template <typename T>
	struct Hash {
		size_t operator()(const T &data, size_t start = 0xcbf29ce484222325ul) const {
			const auto *base = reinterpret_cast<const uint8_t *>(&data);
			for (size_t i = 0; i < sizeof(T); ++i)
				start = (start * 0x00000100000001b3) ^ base[i];
			return start;
		}
	};

	template <size_t BL = 128>
	std::string formatTime(const char *format, std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) {
#ifdef __MINGW32__
		tm now_tm = *localtime(&now);
#else
		tm now_tm;
		localtime_r(&now, &now_tm);
#endif
		std::array<char, BL> buffer;
		strftime(buffer.data(), buffer.size() * sizeof(buffer[0]), format, &now_tm);
		return buffer.data();
	}

	template <typename T, typename R, template <typename...> typename M = std::map, std::floating_point F = double, typename... E>
	const T & weightedChoice(const M<T, F, E...> &map, R &rng) {
		F sum{};
		for (const auto &[item, weight]: map)
			sum += weight;
		F choice = std::uniform_real_distribution<F>(0, sum)(rng);
		F so_far{};
		for (const auto &[item, weight]: map) {
			if (choice < so_far + weight)
				return item;
			so_far += weight;
		}
		throw std::logic_error("Unable to select item from map of weights");
	}

	template <typename O, typename I>
	O safeCast(I input) {
		if (static_cast<I>(std::numeric_limits<O>::max()) < input)
			throw std::invalid_argument("Input number too high: " + std::to_string(input) + " > " + std::to_string(static_cast<I>(std::numeric_limits<O>::max())));

		if constexpr (std::is_signed_v<I> && std::is_signed_v<O>)
			if (static_cast<I>(std::numeric_limits<O>::min()) > input)
				throw std::invalid_argument("Input number too low");

		return static_cast<O>(input);
	}

	struct FreeDeleter {
		void operator()(void *pointer) const noexcept {
			free(pointer);
		}
	};
}
