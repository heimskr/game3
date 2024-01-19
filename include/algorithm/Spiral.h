#pragma once

#include "types/Position.h"

namespace Game3 {
	[[deprecated]] uint64_t getSpiralIndex(const Position &);
	[[deprecated]] Position getSpiralPosition(uint64_t);
}
