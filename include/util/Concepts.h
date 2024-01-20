#pragma once

#include <chrono>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	template <typename T>
	concept Map =
		std::derived_from<T, std::map<typename T::key_type, typename T::mapped_type, typename T::key_compare, typename T::allocator_type>> ||
		std::derived_from<T, std::unordered_map<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal, typename T::allocator_type>>;

	template <typename T>
	concept Set =
		std::derived_from<T, std::set<typename T::value_type, typename T::value_compare, typename T::allocator_type>> ||
		std::derived_from<T, std::unordered_set<typename T::value_type, typename T::hasher, typename T::key_equal, typename T::allocator_type>>;

	template <typename T>
	concept Linear = requires(T t) {
		typename T::value_type;
		t.begin();
		t.end();
		requires !Map<T>;
		requires !Set<T>;
		requires !std::same_as<T, std::string>;
		requires !std::same_as<T, std::string_view>;
		requires !std::same_as<T, nlohmann::json>;
	};

	template <typename T>
	concept LinearOrSet = Set<T> || Linear<T>;

	template <typename T>
	concept Reservable = requires(T t) {
		t.reserve(static_cast<size_t>(UINT32_MAX));
	};

	template <typename T>
	concept Numeric = std::integral<T> || std::floating_point<T>;

	template <typename T, typename R, typename... Args>
	concept Returns = requires(T t, Args &&...args) {
		requires std::is_same_v<std::invoke_result_t<T, Args...>, R>;
	};

	// Credit: https://stackoverflow.com/a/77263021
	template <typename>
	struct is_chrono_duration: std::false_type {};

	template <typename Rep, typename Period>
	struct is_chrono_duration<std::chrono::duration<Rep, Period>>: std::true_type {};

	template <typename T>
	concept Duration = is_chrono_duration<T>::value;
}
