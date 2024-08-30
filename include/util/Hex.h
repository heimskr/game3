#pragma once

#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string_view>

namespace Game3 {
	constexpr uint8_t fromHex(char ch) {
		if ('0' <= ch && ch <= '9')
			return ch - '0';
		if ('a' <= ch && ch <= 'f')
			return ch - 'a' + 10;
		if ('A' <= ch && ch <= 'F')
			return ch - 'A' + 10;
		throw std::invalid_argument("Invalid hex string");
	}

	constexpr uint8_t fromHex(std::string_view pair) {
		if (pair.size() != 2)
			throw std::invalid_argument("Invalid hex pair");
		return (fromHex(pair[0]) << 4) | fromHex(pair[1]);
	}
}
