#pragma once

#include <format>

namespace Game3 {
	struct Rectangle {
		int x{};
		int y{};
		int width{};
		int height{};

		Rectangle(int x, int y, int width, int height):
			x(x), y(y), width(width), height(height) {}

		void scissor() const;
	};
}

template <>
struct std::formatter<Game3::Rectangle> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &rectangle, auto &ctx) const {
		return std::format_to(ctx.out(), "Rectangle({}, {}, {} x {})", rectangle.x, rectangle.y, rectangle.width, rectangle.height);
	}
};