#pragma once

#include <chrono>
#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/json/fwd.hpp>

namespace Game3 {
	template <typename T, template <typename...> typename U>
	concept IsSpecializationOf = requires(std::remove_cvref_t<T> t) {
		// https://www.reddit.com/r/cpp/comments/1hw6a29/a_concept_for_is_specialization_of/m61d6wv/
		[]<typename... Args>(U<Args...> &){}(t);
	};

	template <typename T>
	concept IsDynamicSpan = requires(std::remove_cvref_t<T> t) {
		[]<typename U>(std::span<U, std::dynamic_extent> &){}(t);
	};

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
		requires !std::same_as<T, boost::json::value>;
		requires !IsDynamicSpan<T>;
	};

	template <typename T>
	concept MutableLinear = requires(T t) {
		requires Linear<T>;
		t.push_back(typename T::value_type{});
	};

	template <typename T>
	concept LinearOrSet = Set<T> || Linear<T>;

	template <typename T>
	concept MutableLinearOrSet = requires {
		requires LinearOrSet<T>;
		requires !std::same_as<T, std::string_view>;
	};

	template <typename T>
	concept Reservable = requires(T t) {
		t.reserve(static_cast<size_t>(UINT32_MAX));
	};

	template <typename T, typename R, typename... Args>
	concept Returns = requires(T t, Args &&...args) {
		requires std::is_same_v<std::invoke_result_t<T, Args...>, R>;
	};

	template <typename T, typename Base>
	concept IsDerivedPtr = requires {
		requires std::convertible_to<T, std::shared_ptr<Base>>;
		requires std::derived_from<typename T::element_type, Base>;
	};

	// Credit: https://stackoverflow.com/a/77263021
	template <typename>
	struct is_chrono_duration: std::false_type {};

	template <typename Rep, typename Period>
	struct is_chrono_duration<std::chrono::duration<Rep, Period>>: std::true_type {};

	template <typename T>
	concept Duration = is_chrono_duration<T>::value;

	template <typename T>
	concept EnumClass = !std::is_convertible_v<T, int> && std::is_enum_v<T>;

	template <typename T>
	concept RandomAccessContainer = requires(T t) {
		requires std::random_access_iterator<decltype(t.begin())>;
		requires std::random_access_iterator<decltype(t.end())>;
	};

	template <typename T>
	concept RandomAccessByteContainer = requires(T t) {
		requires RandomAccessContainer<T>;
		requires (sizeof(t[0]) == 1);
	};
}
