#pragma once

#include <cstdint>
#include <string>

namespace Game3 {
	std::string generateFlask(uint16_t hue, float saturation);
	std::string generateFlask(std::string_view hue, std::string_view saturation);
}
