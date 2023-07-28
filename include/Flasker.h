#pragma once

#include <cstdint>
#include <string>

namespace Game3 {
	std::string generateFlask(const std::string &base_filename, const std::string &mask_filename, uint16_t hue, float saturation, float value_diff = 0.f);
	std::string generateFlask(const std::string &base_filename, const std::string &mask_filename, std::string_view hue, std::string_view saturation, std::string_view value_diff);
}
