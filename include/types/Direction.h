#pragma once

#include <array>
#include <cstdint>
#include <format>

namespace Game3 {
	class Buffer;
	struct Position;

	// TODO: basically a duplicate of Quadrant
	enum class Direction: uint8_t {Invalid = 0, Down, Up, Right, Left};

	constexpr std::array<Direction, 4> ALL_DIRECTIONS{Direction::Up, Direction::Right, Direction::Down, Direction::Left};

	Direction remapDirection(Direction, uint16_t configuration);
	Direction rotateClockwise(Direction);
	Direction rotateCounterClockwise(Direction);
	Direction flipDirection(Direction);
	Direction randomDirection();
	bool validateDirection(Direction);
	Position toPosition(Direction);

	std::string toString(Direction);
}

template <>
struct std::formatter<Game3::Direction> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(auto direction, auto &ctx) const {
		switch (direction) {
			case Game3::Direction::Up:      return std::format_to(ctx.out(), "up");
			case Game3::Direction::Down:    return std::format_to(ctx.out(), "down");
			case Game3::Direction::Left:    return std::format_to(ctx.out(), "left");
			case Game3::Direction::Right:   return std::format_to(ctx.out(), "right");
			case Game3::Direction::Invalid: return std::format_to(ctx.out(), "invalid");
			default:
				return std::format_to(ctx.out(), "{}?", int(direction));
		}
	}
};
