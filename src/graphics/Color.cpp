#include "graphics/Color.h"
#include "lib/ok_color.h"
#include "util/Util.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace Game3 {
	RGB::RGB(std::string_view str) {
		if (!str.empty() && str[0] == '#')
			str.remove_prefix(1);

		if (str.size() != 6)
			throw std::invalid_argument("Invalid RGB string length: expected 6");

		r = fromHex(str.substr(0, 2)) / 255.f;
		g = fromHex(str.substr(2, 2)) / 255.f;
		b = fromHex(str.substr(4, 2)) / 255.f;
	}

	template <>
	RGB convertColor(const OKHsv &hsv) {
		ok_color::RGB rgb = ok_color::okhsv_to_srgb({hsv.h, hsv.s, hsv.v});
		return {rgb.r, rgb.g, rgb.b};
	}

	template <>
	OKHsv convertColor(const RGB &rgb) {
		ok_color::HSV hsv = ok_color::srgb_to_okhsv({rgb.r, rgb.g, rgb.b});
		return {hsv.h, hsv.s, hsv.v};
	}

	OKHsv OKHsv::darken() const {
		auto [hue, saturation, value] = *this;

		if (60.f / 360.f <= hue && hue < 270.f / 360.f) {
			hue += 15.f / 360.f;
		} else {
			hue -= 15.f / 360.f;
			if (hue < 0.f)
				hue += 1.f;
		}

		saturation = std::min(1.f, saturation + .1f);

		value *= 2.f / 3.f;

		return OKHsv{hue, saturation, value};
	}

	RGB RGB::darken() const {
		return convert<OKHsv>().darken().convert<RGB>();
	}
}
