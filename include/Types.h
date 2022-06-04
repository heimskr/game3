#pragma once

#include <cstdint>

namespace Game3 {
	using TileID = uint8_t;
	using Index = unsigned;
	using PlayerID = int;
	using EntityID = unsigned;
	using RealmID = unsigned;
	using ItemID = unsigned;
	using Slot = int;

	enum class Direction: uint8_t {Up, Right, Down, Left};
}
