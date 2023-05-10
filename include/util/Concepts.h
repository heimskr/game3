#pragma once

#include <climits>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Game3 {
	template <typename T>
	concept Map =
		std::same_as<T, std::map<typename T::key_type, typename T::mapped_type, typename T::key_compare, typename T::allocator_type>> ||
		std::same_as<T, std::unordered_map<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal, typename T::allocator_type>>;

	template <typename T>
	concept Set =
		std::same_as<T, std::set<typename T::value_type, typename T::value_compare, typename T::allocator_type>> ||
		std::same_as<T, std::unordered_set<typename T::value_type, typename T::hasher, typename T::key_equal, typename T::allocator_type>>;

	template <typename T>
	concept Linear = requires(T t) {
		typename T::value_type;
		t.begin();
		t.end();
		requires !Map<T>;
		requires !Set<T>;
	};

	template <typename T>
	concept LinearOrSet = Set<T> || Linear<T>;

	template <typename T>
	concept Reservable = requires(T t) {
		t.reserve(static_cast<size_t>(UINT32_MAX));
	};

	template <typename T>
	concept Numeric = std::integral<T> || std::floating_point<T>;
}
