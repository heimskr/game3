#pragma once

#include <cstdint>
#include <ostream>

namespace Game3 {
	class Buffer;

	// TODO: basically a duplicate of Quadrant
	enum class Direction: uint8_t {Invalid = 0, Down, Up, Right, Left};

	Direction remapDirection(Direction, uint16_t configuration);
	Direction flipDirection(Direction);
	Direction randomDirection();

	std::ostream & operator<<(std::ostream &, Direction);
}
