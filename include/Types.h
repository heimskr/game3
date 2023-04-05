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
	using RealmID      =  int32_t;
	using ItemID       = uint32_t;
	using Slot         =  int32_t;
	using ItemCount    = uint64_t;
	using MoneyCount   = uint64_t;
	using Phase        =  uint8_t;
	using Durability   =  int32_t;
	using BiomeType    = uint32_t;
	/** Number of quarter-hearts. */
	using HitPoints    = uint32_t;

	enum class CraftingStationType {None, Furnace, Anvil, Cauldron};
	enum class Ore {Coal, Copper, Iron, Gold, Diamond};

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	struct Place;
	using PlacePtr = std::shared_ptr<Place>;
}
