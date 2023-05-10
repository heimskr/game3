#pragma once

#include <cstdint>
#include <ostream>

namespace Game3 {
	class Buffer;

	enum class Direction: uint8_t {Down = 0, Up, Right, Left};

	Direction remapDirection(Direction, uint16_t configuration);
	Direction randomDirection();

	std::ostream & operator<<(std::ostream &, Direction);
	// Buffer & operator+=(Buffer &, Direction);
	// Buffer & operator<<(Buffer &, Direction);
	// Buffer & operator>>(Buffer &, Direction &);
}
