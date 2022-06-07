#pragma once

#include <ostream>

namespace Game3 {
	enum class Direction: uint8_t {Down = 0, Up, Right, Left};
}

std::ostream & operator<<(std::ostream &, Game3::Direction);
