#pragma once

#include <cstddef>
#include <cstdint>
#include <format>
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
	using LaborAmount    = double;

	using ItemID     = Identifier;
	using EntityType = Identifier;
	using RealmType  = Identifier;

	class Game;
	using GamePtr = std::shared_ptr<Game>;

	class ClientGame;
	using ClientGamePtr = std::shared_ptr<ClientGame>;

	class ServerGame;
	using ServerGamePtr = std::shared_ptr<ServerGame>;

	class Agent;
	using AgentPtr = std::shared_ptr<Agent>;

	class Entity;
	using EntityPtr = std::shared_ptr<Entity>;

	class TileEntity;
	using TileEntityPtr = std::shared_ptr<TileEntity>;

	class Player;
	using PlayerPtr = std::shared_ptr<Player>;

	class ServerPlayer;
	using ServerPlayerPtr = std::shared_ptr<ServerPlayer>;

	class ClientPlayer;
	using ClientPlayerPtr = std::shared_ptr<ClientPlayer>;

	class Inventory;
	using InventoryPtr = std::shared_ptr<Inventory>;

	class ItemStack;
	using ItemStackPtr = std::shared_ptr<ItemStack>;
	using ConstItemStackPtr = std::shared_ptr<const ItemStack>;

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

	Buffer & operator+=(Buffer &, const Color &);
	Buffer & operator<<(Buffer &, const Color &);
	Buffer & operator>>(Buffer &, Color &);

	enum class Substance: uint8_t {Invalid = 0, Item, Fluid, Energy, Data};
	enum class Hand: uint8_t {None = 0, Left, Right};
}

template <>
struct std::formatter<Game3::Substance> {
	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(auto pipe_type, std::format_context &ctx) const {
		switch (pipe_type) {
			case Game3::Substance::Item:   return std::format_to(ctx.out(), "Item");
			case Game3::Substance::Fluid:  return std::format_to(ctx.out(), "Fluid");
			case Game3::Substance::Energy: return std::format_to(ctx.out(), "Energy");
			case Game3::Substance::Data:   return std::format_to(ctx.out(), "Data");
			default:
				return std::format_to(ctx.out(), "Invalid");
		}
	}
};

template <>
struct std::formatter<Game3::Hand> {
	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &hand, std::format_context &ctx) const {
		switch (hand) {
			case Game3::Hand::None:  return std::format_to(ctx.out(), "None");
			case Game3::Hand::Left:  return std::format_to(ctx.out(), "Left");
			case Game3::Hand::Right: return std::format_to(ctx.out(), "Right");
			default:
				return std::format_to(ctx.out(), "Invalid");
		}
	}
};

template <>
struct std::formatter<Game3::Color> {
	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
	}

	auto format(const auto &color, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "({}, {}, {} @ {})", color.red, color.green, color.blue, color.alpha);
	}
};
