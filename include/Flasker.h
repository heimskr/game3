#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

namespace Game3 {
	std::unique_ptr<uint8_t[]> generateFlaskRaw(const std::filesystem::path &base_filename, const std::filesystem::path &mask_filename, uint16_t hue, float saturation, float value_diff = 0.f, int *width_out = nullptr, int *height_out = nullptr);
	std::string generateFlask(const std::filesystem::path &base_filename, const std::filesystem::path &mask_filename, uint16_t hue, float saturation, float value_diff = 0.f);
	std::string generateFlask(const std::filesystem::path &base_filename, const std::filesystem::path &mask_filename, std::string_view hue, std::string_view saturation, std::string_view value_diff);
}
