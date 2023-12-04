#pragma once

#include "types/Position.h"
#include "types/Types.h"

#include <optional>

namespace Game3 {
	struct MovementContext {
		bool excludePlayerSelf = false;
		bool fromPath = false;
		bool clearOffset = true;
		std::optional<Direction> facingDirection{};
		std::optional<Position> forcedPosition{};
		std::optional<Vector3> forcedOffset{};
		bool isTeleport = false;
	};
}
