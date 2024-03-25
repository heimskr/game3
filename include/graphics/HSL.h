#pragma once

#include <cstdint>

namespace Game3 {
	struct HSL {
		uint16_t h{};
		float s{};
		float l{};
		uint8_t a{};

		HSL() = default;

		HSL(uint16_t h_, float s_, float l_, uint8_t a_ = 255):
			h(h_), s(s_), l(l_), a(a_) {}
	};
}
