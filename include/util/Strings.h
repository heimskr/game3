#pragma once

#include "math/Concepts.h"

#include <charconv>
#include <format>
#include <stdexcept>
#include <string>

namespace Game3 {
	template <typename T>
	inline T fromString(std::string_view string) {
		return T(string);
	}

	template <Numeric T>
	T fromString(std::string_view string) {
		T out{};
		std::from_chars_result result = std::from_chars(string.begin(), string.end(), out, 10);
		if (result.ec != std::errc{} || result.ptr - string.begin() != std::ssize(string)) {
			throw std::invalid_argument(std::format("Not an integer: \"{}\"", string));
		}
		return out;
	}

	template <typename T>
    requires (!Numeric<T>)
	inline std::string toString(const T &value) {
		return std::string(value);
	}

	template <Numeric T>
	inline std::string toString(T value) {
		return std::to_string(value);
	}

	inline const std::string & toString(const std::string &string) {
		return string;
	}

	inline std::string_view toString(std::string_view string) {
		return string;
	}
}
