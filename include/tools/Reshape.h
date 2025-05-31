#pragma once

#include <filesystem>
#include <vector>

namespace Game3 {
	std::vector<uint8_t> reshape47(std::span<const uint8_t> png_bytes, uint32_t tile_size);
	std::vector<uint8_t> reshape47(const std::string &png_bytes, uint32_t tile_size);
}
