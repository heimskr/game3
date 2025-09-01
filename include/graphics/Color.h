#pragma once

#include "util/Hex.h"
#include "util/Math.h"

#include <boost/json/fwd.hpp>

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

		constexpr Color(std::string_view string) {
			if (string.empty()) {
				throw std::invalid_argument("Color string must not be empty");
			}

			if (string[0] == '#') {
				string.remove_prefix(1);
			}

			if (string.empty()) {
				// "#" -> transparent black
				alpha = 0;
				return;
			}

			if (string.size() == 2) {
				// "#xx" -> variable-opacity black
				red = 0;
				green = 0;
				blue = 0;
				alpha = fromHex(string) / 255.f;
				return;
			}

			if (string.size() == 3 || string.size() == 4) {
				// #rgb -> #rrggbb, #rgba -> #rrggbbaa
				auto get = [](char character) -> float {
					const uint8_t hexed = fromHex(character);
					return (hexed | (hexed << 4)) / 255.f;
				};

				red = get(string[0]);
				green = get(string[1]);
				blue = get(string[2]);

				if (string.size() == 4) {
					alpha = get(string[3]);
				}

				return;
			}

			if (string.size() != 6 && string.size() != 8) {
				throw std::invalid_argument("Invalid size for color string");
			}

			auto get = [&](size_t offset) -> float {
				return string.empty()? 1.f : fromHex(string.substr(offset, 2)) / 255.f;
			};

			red = get(0);
			green = get(2);
			blue = get(4);
			string.remove_prefix(6);
			alpha = get(0);
		}

		constexpr Color(const char *string):
			Color(std::string_view(string)) {}

		Color darken(float value_divisor = 3.f / 2.f) const;
		Color multiplyValue(float multiplier) const;
		Color invertValue() const;
		Color desaturate() const;
		Color withAlpha(float) const;

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

	class BasicBuffer;
	class Buffer;

	Buffer & operator+=(Buffer &, const Color &);
	Buffer & operator<<(Buffer &, const Color &);
	BasicBuffer & operator>>(BasicBuffer &, Color &);

	Color tag_invoke(boost::json::value_to_tag<Color>, const boost::json::value &);
}

template <>
struct std::formatter<Game3::Color> {
	bool hash = false;

	constexpr auto parse(auto &ctx) {
		auto iter = ctx.begin();
		if (*iter == '#') {
			hash = true;
			++iter;
		}
		return iter;
	}

	auto format(const auto &color, auto &ctx) const {
		if (hash) {
			std::array<char, 10> buffer{'#'};

			auto to_hex = [&](float f, char offset) {
				auto to_char = [](uint8_t nybble) {
					return nybble < 10? '0' + nybble : 'a' + (nybble - 10);
				};

				const uint8_t byte(f * 255);
				buffer[offset] = to_char(byte >> 4);
				buffer[offset + 1] = to_char(byte & 0xf);
			};

			to_hex(color.red,   1);
			to_hex(color.green, 3);
			to_hex(color.blue,  5);
			if (color.alpha < 1.f) {
				to_hex(color.alpha, 7);
			}

			return std::format_to(ctx.out(), "{}", buffer.data());
		}

		return std::format_to(ctx.out(), "({}, {}, {} @ {})", color.red, color.green, color.blue, color.alpha);
	}
};
