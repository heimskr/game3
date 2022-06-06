#pragma once

#include <cstdint>
#include <utility>

namespace Game3 {
	using TileID = uint8_t;
	using Index = int;
	using PlayerID = int;
	using EntityID = unsigned;
	using RealmID = unsigned;
	using ItemID = unsigned;
	using Slot = int;

	enum class Direction: uint8_t {Down = 0, Up, Right, Left};
}
