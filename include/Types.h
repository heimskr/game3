#pragma once

#include <cstdint>
#include <utility>

namespace Game3 {
	using Index        =  int32_t;
	using TileID       = uint16_t;
	using PlayerID     =  int32_t;
	using TileEntityID = uint32_t;
	using EntityType   = uint32_t;
	using RealmType    = uint32_t;
	using EntityID     = uint32_t;
	using RealmID      = uint32_t;
	using ItemID       = uint32_t;
	using Slot         =  int32_t;
	using ItemCount    = uint64_t;
	using MoneyCount   = uint64_t;
	using Phase        =  uint8_t;
	using Durability   =  int32_t;

	enum class CraftingStationType {None, Furnace, Anvil};
}
