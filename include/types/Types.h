#pragma once

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <shared_mutex>
#include <utility>

#include "data/Identifier.h"
#include "registry/Registerable.h"
#include "util/Math.h"

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
	using BiomeType      = uint16_t;
	/** Number of quarter-hearts. */
	using HitPoints      =  int32_t;
	/** 1-based. */
	using PacketID       = uint16_t;
	using Version        = uint32_t;
	using GlobalID       = uint64_t;
	using Token          = uint64_t;
	using FluidLevel     = uint16_t;
	using FluidAmount    = uint64_t;
	using FluidID        = uint16_t;
	using UpdateCounter  = uint64_t;
	using Tick           = uint64_t;
	using EnergyAmount   = uint64_t;
	using InventoryID    = uint16_t;
	using VillageID      = uint64_t;
	using LaborAmount    = uint32_t;

	using ItemID     = Identifier;
	using EntityType = Identifier;
	using RealmType  = Identifier;

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	class ServerPlayer;
	using ServerPlayerPtr = std::shared_ptr<ServerPlayer>;

	class ClientPlayer;
	using ClientPlayerPtr = std::shared_ptr<ClientPlayer>;

	using Resources = std::map<Identifier, double>;

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

		constexpr Color() = default;
		constexpr Color(float red_, float green_, float blue_, float alpha_ = 1.f):
			red(red_), green(green_), blue(blue_), alpha(alpha_) {}

		explicit constexpr Color(uint32_t packed):
			red((packed >> 24) / 255.f),
			green(((packed >> 16) & 0xff) / 255.f),
			blue(((packed >> 8) & 0xff) / 255.f),
			alpha((packed & 0xff) / 255.f) {}

		static Color fromBytes(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);
	};

	constexpr Color lerp(const Color &from, const Color &to, float progress) {
		return Color{
			lerp(from.red,   to.red,   progress),
			lerp(from.green, to.green, progress),
			lerp(from.blue,  to.blue,  progress),
			lerp(from.alpha, to.alpha, progress),
		};
	}

	class Buffer;

	std::ostream & operator<<(std::ostream &, const Color &);
	Buffer & operator+=(Buffer &, const Color &);
	Buffer & operator<<(Buffer &, const Color &);
	Buffer & operator>>(Buffer &, Color &);

	enum class PipeType: uint8_t {Invalid = 0, Item, Fluid, Energy};
	std::ostream & operator<<(std::ostream &, PipeType);

	enum class Hand: uint8_t {None = 0, Left, Right};
	std::ostream & operator<<(std::ostream &, Hand);
}
