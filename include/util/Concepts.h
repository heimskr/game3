#pragma once

#include <deque>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>

namespace Game3 {
	template <typename T>
	concept Map =
		std::same_as<T, std::map<typename T::key_type, typename T::mapped_type, typename T::key_compare, typename T::allocator_type>> ||
		std::same_as<T, std::unordered_map<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal, typename T::allocator_type>>;

	template <typename T>
	concept LinearContainer = requires(T t) {
		typename T::value_type;
		requires !Map<T>;
	};
}
