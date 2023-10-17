#pragma once

#include <array>
#include <cstdint>
#include <ostream>

namespace Game3 {
	class Buffer;

	// TODO: basically a duplicate of Quadrant
	enum class Direction: uint8_t {Invalid = 0, Down, Up, Right, Left};

	constexpr std::array<Direction, 4> ALL_DIRECTIONS{Direction::Up, Direction::Right, Direction::Down, Direction::Left};

	Direction remapDirection(Direction, uint16_t configuration);
	Direction rotateClockwise(Direction);
	Direction rotateCounterClockwise(Direction);
	Direction flipDirection(Direction);
	Direction randomDirection();
	bool validateDirection(Direction);

	std::string toString(Direction);
	std::ostream & operator<<(std::ostream &, Direction);
}
