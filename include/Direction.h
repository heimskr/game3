#pragma once

#include <ostream>

namespace Game3 {
	enum class Direction: uint8_t {Down = 0, Up, Right, Left};

	Direction remapDirection(Direction, uint16_t configuration);
}

std::ostream & operator<<(std::ostream &, Game3::Direction);
