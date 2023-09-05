#pragma once

#include <bit>
#include <cstdint>
#include <span>

namespace Game3 {
	constexpr inline bool isLittleEndian() {
		return std::endian::native == std::endian::little;
	}

	int32_t decodeLittleS32(std::span<const char>);
}
