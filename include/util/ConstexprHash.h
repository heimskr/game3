#pragma once

#include <cstddef>
#include <cstdint>

namespace Game3 {
	constexpr uint32_t computeFNV1A(const char *string, size_t i) {
		return ((i > 0? computeFNV1A(string, i - 1) : 0x811c9dc5) ^ static_cast<uint32_t>(string[i])) * 0x1000193;
	}

	constexpr uint32_t operator""_fnv(const char *string, size_t size) {
		return computeFNV1A(string, size);
	}
}
