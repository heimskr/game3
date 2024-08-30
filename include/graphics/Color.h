#pragma once

#include "util/Hex.h"
#include "util/Math.h"

#include <format>
#include <string_view>

namespace Game3 {
	template <typename Src, typename Dest>
	Dest convertColor(const Src &);

	template <typename T>
	struct BaseColor {
		template <typename U>
		U convert() const {
			return convertColor<T, U>(static_cast<const T &>(*this));
		}
	};

	struct OKHsv: BaseColor<OKHsv> {
		float hue = 0;
		float saturation = 0;
		float value = 0;
		float alpha = 1;

		OKHsv(float hue, float saturation, float value, float alpha):
			hue(hue), saturation(saturation), value(value), alpha(alpha) {}

		OKHsv darken(float value_divisor) const;
	};

	struct Color: BaseColor<Color> {
		float red   = 0;
		float green = 0;
		float blue  = 0;
		float alpha = 1;

		constexpr Color() = default;
		constexpr Color(float red_, float green_, float blue_, float alpha_ = 1.f):
			red(red_), green(green_), blue(blue_), alpha(alpha_) {}

		explicit constexpr Color(uint32_t packed):
			red((packed >> 24) / 255.f),
			green(((packed >> 16) & 0xff) / 255.f),
			blue(((packed >> 8) & 0xff) / 255.f),
			alpha((packed & 0xff) / 255.f) {}

		explicit constexpr Color(std::string_view str) {
			if (str.empty())
				throw std::invalid_argument("Color string must not be empty");

			if (str[0] == '#')
				str.remove_prefix(1);

			if (str.size() != 6 && str.size() != 8)
				throw std::invalid_argument("Invalid size for color string");

			red = fromHex(str.substr(0, 2)) / 255.f;
			green = fromHex(str.substr(2, 2)) / 255.f;
			blue = fromHex(str.substr(4, 2)) / 255.f;
			str.remove_prefix(6);
			alpha = str.empty()? 1.f : fromHex(str) / 255.f;
		}

		Color darken(float value_divisor = 3.f / 2.f) const;
		Color multiplyValue(float multiplier) const;
		Color invertValue() const;
		Color desaturate() const;

		static Color fromBytes(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);
	};

	constexpr Color lerp(const Color &from, const Color &to, float progress) {
		return Color{
			lerp(from.red,   to.red,   progress),
			lerp(from.green, to.green, progress),
			lerp(from.blue,  to.blue,  progress),
			lerp(from.alpha, to.alpha, progress),
		};
	}

	class Buffer;

	Buffer & operator+=(Buffer &, const Color &);
	Buffer & operator<<(Buffer &, const Color &);
	Buffer & operator>>(Buffer &, Color &);
}

template <>
struct std::formatter<Game3::Color> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &color, auto &ctx) const {
		return std::format_to(ctx.out(), "({}, {}, {} @ {})", color.red, color.green, color.blue, color.alpha);
	}
};
