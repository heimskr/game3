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

		Rectangle(int x, int y):
			Rectangle(x, y, 0, 0) {}

		int area() const;
		void scissor(int outer_height) const;
		void viewport(int outer_height) const;
		bool contains(int x, int y) const;
		void reposition(int x, int y) &;
		Rectangle && reposition(int x, int y) &&;
		Rectangle intersection(const Rectangle &) const;

		auto operator<=>(const Rectangle &) const = default;

		/** Ignores the width/height of the LHS and uses the width/height of the RHS. */
		Rectangle operator+(const Rectangle &) const;

		/** Ignores the width/height of the RHS and uses the width/height of the LHS. */
		Rectangle operator-(const Rectangle &) const;

		/** Returns whether both the height and width of the rectangle are positive. */
		operator bool() const;
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
