#pragma once

#include "types/Position.h"

#include <format>

namespace Game3 {
	struct Rectangle {
		int x{};
		int y{};
		int width{};
		int height{};

		Rectangle() = default;

		constexpr Rectangle(int x, int y, int width, int height):
			x(x), y(y), width(width), height(height) {}

		constexpr Rectangle(int x, int y):
			Rectangle(x, y, 0, 0) {}

		Rectangle(Vector2d pos, int size):
			Rectangle(pos.x, pos.y, size, size) {}

		Rectangle(Vector2i pos, int size):
			Rectangle(pos.x, pos.y, size, size) {}

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

		Rectangle shrinkTop(int) const;
		Rectangle shrinkRight(int) const;
		Rectangle shrinkBottom(int) const;
		Rectangle shrinkLeft(int) const;
		Rectangle shrinkAll(int) const;

		template <typename T>
		Rectangle operator*(T scale) const {
			return Rectangle(x, y, width * scale, height * scale);
		}

		/** Returns whether both the height and width of the rectangle are positive. */
		operator bool() const;

		std::pair<int, int> size() const {
			return {width, height};
		}
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
