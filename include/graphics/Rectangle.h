#pragma once

#include <format>

namespace Game3 {
	struct Rectangle {
		int x{};
		int y{};
		int width{};
		int height{};

		Rectangle() = default;

		Rectangle(int x, int y, int width, int height):
			x(x), y(y), width(width), height(height) {}


		void scissor() const;
		bool contains(int x, int y) const;

		auto operator<=>(const Rectangle &) const = default;
		/** Ignores the width/height of the LHS and uses the width/height of the RHS. */
		Rectangle operator+(const Rectangle &) const;
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
