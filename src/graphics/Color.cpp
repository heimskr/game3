#include "graphics/Color.h"
#include "lib/ok_color.h"
#include "net/Buffer.h"
#include "util/Util.h"

#include <boost/json.hpp>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace Game3 {
	template <>
	Color convertColor(const OKHsv &hsv) {
		ok_color::RGB rgb = ok_color::okhsv_to_srgb({hsv.hue, hsv.saturation, hsv.value});
		return {rgb.r, rgb.g, rgb.b, hsv.alpha};
	}

	template <>
	OKHsv convertColor(const Color &rgb) {
		// Horrible NaN nonsense abounds.
		if (rgb.red < 0.001 && rgb.green < 0.001 && rgb.blue < 0.001) {
			return {0, 0, 0, rgb.alpha};
		}

		if (rgb.red > 0.999 && rgb.green > 0.999 && rgb.blue > 0.999) {
			return {1, 1, 1, rgb.alpha};
		}

		ok_color::HSV hsv = ok_color::srgb_to_okhsv({rgb.red, rgb.green, rgb.blue});
		return {hsv.h, hsv.s, hsv.v, rgb.alpha};
	}

	OKHsv OKHsv::darken(float value_divisor) const {
		auto [hue, saturation, value, alpha] = *this;

		if (60.f / 360.f <= hue && hue < 270.f / 360.f) {
			hue += 15.f / 360.f;
		} else {
			hue -= 15.f / 360.f;
			if (hue < 0.f)
				hue += 1.f;
		}

		saturation = std::min(1.f, saturation + .1f);

		value /= value_divisor;

		return {hue, saturation, value, alpha};
	}

	Color Color::darken(float value_divisor) const {
		return convert<OKHsv>().darken(value_divisor).convert<Color>();
	}

	Color Color::multiplyValue(float multiplier) const {
		OKHsv ok = convert<OKHsv>();
		ok.value = std::min(ok.value * multiplier, 1.f);
		return ok.convert<Color>();
	}

	Color Color::invertValue() const {
		OKHsv ok = convert<OKHsv>();
		ok.value = 1.f - ok.value;
		return ok.convert<Color>();
	}

	Color Color::desaturate() const {
		OKHsv ok = convert<OKHsv>();
		ok.saturation = 0;
		return ok.convert<Color>();
	}

	Color Color::withAlpha(float new_alpha) const {
		return {red, green, blue, new_alpha};
	}

	Color Color::fromBytes(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
		return {red / 255.f, green / 255.f, blue / 255.f, alpha / 255.f};
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

	Color tag_invoke(boost::json::value_to_tag<Color>, const boost::json::value &json) {
		return Color(json.as_string());
	}
}
