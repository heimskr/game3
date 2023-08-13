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

		// explicit MovementContext(bool exclude_player_self = false, bool from_path = false, bool clear_offset = true):
		// 	excludePlayerSelf(exclude_player_self), fromPath(from_path), clearOffset(clear_offset) {}
	};
}
