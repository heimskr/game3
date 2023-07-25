#pragma once

#include <cstdint>
#include <string>

namespace Game3 {
	std::string generateFlask(uint16_t hue, float saturation, float value_diff = 0.f);
	std::string generateFlask(std::string_view hue, std::string_view saturation, std::string_view value_diff);
}
