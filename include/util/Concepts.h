#pragma once

#include <map>
#include <unordered_map>

namespace Game3 {
	template<typename T>
	concept map_type =
		std::same_as<T, std::map<typename T::key_type, typename T::mapped_type, typename T::key_compare, typename T::allocator_type>> ||
		std::same_as<T, std::unordered_map<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal, typename T::allocator_type>>;
}
