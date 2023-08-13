#pragma once

#include "Position.h"
#include "Types.h"

#include <optional>

namespace Game3 {
	struct MovementContext {
		bool excludePlayerSelf = false;
		bool fromPath = false;
		bool clearOffset = true;
		std::optional<Direction> facingDirection;
		std::optional<Position> forcedPosition;
		std::optional<Offset> forcedOffset;
	};
}
