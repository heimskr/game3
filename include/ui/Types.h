#pragma once

#include <format>

namespace Game3 {
	enum class Orientation: uint8_t {Neither, Vertical, Horizontal, Both};
	enum class SizeRequestMode: uint8_t {HeightForWidth, WidthForHeight, ConstantSize, Expansive};
	enum class Alignment: uint8_t {Start, Center, End};
	enum class ButtonsType: uint8_t {None, Cancel, Okay, CancelOkay, No, Yes, NoYes};
	enum class Expansion: uint8_t {None, Shrink, Expand};
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
