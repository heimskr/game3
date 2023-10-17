#pragma once

#include "types/Direction.h"
#include "types/Position.h"

namespace Game3 {
	struct DirectedPlace: Place {
		Direction direction;

		template <typename... Args>
		DirectedPlace(Direction direction_, Args &&...args):
			Place(std::forward<Args>(args)...),
			direction(direction_) {}
	};
}
