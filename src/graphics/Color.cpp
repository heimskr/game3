#include "graphics/Color.h"
#include "lib/ok_color.h"
#include "net/Buffer.h"
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

	Color Color::fromBytes(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
		return Color{red / 255.f, green / 255.f, blue / 255.f, alpha / 255.f};
	}

	template <>
	std::string Buffer::getType(const Color &, bool) {
		return std::string{'\x33'} + getType(float{}, false);
	}

	Buffer & operator+=(Buffer &buffer, const Color &color) {
		return (((buffer.appendType(color, false) += color.red) += color.green) += color.blue) += color.alpha;
	}

	Buffer & operator<<(Buffer &buffer, const Color &color) {
		return buffer += color;
	}

	Buffer & operator>>(Buffer &buffer, Color &color) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(color, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected shortlist<f32, 4> for Color)");
		}
		popBuffer(buffer, color.red);
		popBuffer(buffer, color.green);
		popBuffer(buffer, color.blue);
		popBuffer(buffer, color.alpha);
		return buffer;
	}
}
