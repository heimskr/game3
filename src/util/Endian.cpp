#include "util/Endian.h"

#include <cstring>
#include <stdexcept>

namespace Game3 {
	int32_t decodeLittleS32(std::span<const char> span) {
		if (span.size() < sizeof(int32_t))
			throw std::invalid_argument("Invalid span size in decodeLittleS32");

		if constexpr (isLittleEndian()) {
			// TODO: Is this actually faster, especially with -Og?
			int32_t out{};
			std::memcpy(&out, span.data(), sizeof(out));
			return out;
		}

		return span[0] | (int32_t(span[1]) << 8) | (int32_t(span[2]) << 16) | (int32_t(span[3]) << 24);
	}
}
