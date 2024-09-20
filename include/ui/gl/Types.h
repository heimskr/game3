#pragma once

#include <format>

namespace Game3 {
	enum class Orientation: char {Vertical, Horizontal};
	enum class SizeRequestMode: char {HeightForWidth, WidthForHeight, ConstantSize, Expansive};
	enum class Alignment: char {Start, Center, End};
}

template <>
struct std::formatter<Game3::Orientation> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const Game3::Orientation &orientation, auto &ctx) const {
		if (orientation == Game3::Orientation::Vertical)
			return std::format_to(ctx.out(), "Vertical");

		if (orientation == Game3::Orientation::Horizontal)
			return std::format_to(ctx.out(), "Horizontal");

		return std::format_to(ctx.out(), "Orientation[{}]?", static_cast<int>(orientation));
	}
};
