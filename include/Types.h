#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include "data/Identifier.h"
#include "registry/Registerable.h"

namespace Game3 {
	using Index        =  int64_t;
	using TileID       = uint16_t;
	using PlayerID     =  int32_t;
	using RealmID      =  int32_t;
	using Slot         =  int32_t;
	using ItemCount    = uint64_t;
	using MoneyCount   = uint64_t;
	using Phase        =  uint8_t;
	using Durability   =  int32_t;
	using BiomeType    = uint32_t;
	/** Number of quarter-hearts. */
	using HitPoints    = uint32_t;

	// enum class CraftingStationType {None, Furnace, Anvil, Cauldron, Purifier};
	// enum class Ore {Coal, Copper, Iron, Gold, Diamond};

	using ItemID     = Identifier;
	using EntityType = Identifier;
	using RealmType  = Identifier;

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	struct Place;
	using PlacePtr = std::shared_ptr<Place>;

	template <typename T>
	class NamedNumeric: public NamedRegisterable {
		public:
			NamedNumeric(Identifier identifier_, T value_):
				NamedRegisterable(std::move(identifier_)),
				value(value_) {}

			operator T() const { return value; }

		private:
			T value;
	};

	struct NamedDurability: NamedNumeric<Durability> {
		using NamedNumeric::NamedNumeric;
	};
}
