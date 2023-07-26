#pragma once

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <utility>

#include "data/Identifier.h"
#include "registry/Registerable.h"

namespace Game3 {
	using Index          =  int64_t;
	using TileID         = uint16_t;
	using PlayerID       =  int32_t;
	using RealmID        =  int32_t;
	using Slot           =  int32_t;
	using ItemCount      = uint64_t;
	using MoneyCount     = uint64_t;
	using Phase          =  uint8_t;
	using Durability     =  int32_t;
	using BiomeType      = uint32_t;
	/** Number of quarter-hearts. */
	using HitPoints      = uint32_t;
	/** 1-based. */
	using PacketID       = uint16_t;
	using Version        = uint32_t;
	using GlobalID       = uint64_t;
	using Token          = uint64_t;
	using FluidLevel     = uint16_t;
	using FluidAmount = uint64_t;
	using FluidID        = uint16_t;
	using UpdateCounter  = uint64_t;
	using Tick           = uint64_t;

	using ItemID     = Identifier;
	using EntityType = Identifier;
	using RealmType  = Identifier;

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	class ServerPlayer;
	using ServerPlayerPtr = std::shared_ptr<ServerPlayer>;

	class ClientPlayer;
	using ClientPlayerPtr = std::shared_ptr<ClientPlayer>;

	struct Place;

	enum class Side {Invalid = 0, Server, Client};

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

	Index operator""_idx(unsigned long long);

	enum class PathResult: uint8_t {Invalid, Trivial, Unpathable, Success};

	struct Color {
		float red   = 0.f;
		float green = 0.f;
		float blue  = 0.f;
		float alpha = 1.f;

		Color() = default;
		Color(float red_, float green_, float blue_, float alpha_ = 1.f):
			red(red_), green(green_), blue(blue_), alpha(alpha_) {}
	};

	enum class PipeType {Item, Fluid, Energy};
	std::ostream & operator<<(std::ostream &, PipeType);
}
