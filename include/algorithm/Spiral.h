#pragma once

#include "types/Position.h"

namespace Game3 {
	uint64_t getSpiralIndex(const Position &);
	Position getSpiralPosition(uint64_t);
}
