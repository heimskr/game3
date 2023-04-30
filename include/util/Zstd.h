#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace Game3 {
	std::vector<uint8_t>  decompress8 (std::span<const uint8_t> span);
	std::vector<uint16_t> decompress16(std::span<const uint8_t> span);
	std::vector<uint32_t> decompress32(std::span<const uint8_t> span);
	std::vector<uint8_t> compress(std::span<const uint8_t > span);
	std::vector<uint8_t> compress(std::span<const uint16_t> span);
	std::vector<uint8_t> compress(std::span<const uint32_t> span);
}
