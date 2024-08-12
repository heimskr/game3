#pragma once

#include "types/Direction.h"
#include "Layer.h"
#include "types/Types.h"

#include <cmath>
#include <functional>
#include <memory>
#include <optional>
#include <ostream>
#include <string>

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Buffer;
	class Game;
	class Realm;
	class TileEntity;
	struct ChunkPosition;

	struct Position {
		using IntType = Index;
		IntType row = 0;
		IntType column = 0;

		Position() = default;
		Position(IntType row_, IntType column_): row(row_), column(column_) {}
		Position(std::string_view);

		inline bool operator==(const Position &other) const { return this == &other || (row == other.row && column == other.column); }
		inline bool operator!=(const Position &other) const { return this != &other && (row != other.row || column != other.column); }
		inline Position operator+(const Position &other) const { return {row + other.row, column + other.column}; }
		inline Position operator-(const Position &other) const { return {row - other.row, column - other.column}; }
		inline Position & operator+=(const Position &other) { row += other.row; column += other.column; return *this; }
		inline Position & operator-=(const Position &other) { row -= other.row; column -= other.column; return *this; }
		Position operator+(Direction) const;
		Position operator-(Direction) const;
		Position operator*(Index) const;
		Position operator/(Index) const;
		Position operator*(double) const;
		Position operator/(double) const;
		Position & operator+=(Direction);
		Position & operator-=(Direction);
		Position & operator*=(Index);
		Position & operator/=(Index);
		Position & operator*=(double);
		Position & operator/=(double);
		ChunkPosition getChunk() const;
		/** Has a bias for horizontal directions when the difference is diagonal. */
		Direction getFacing(const Position &other) const;
		explicit inline operator std::string() const { return '(' + std::to_string(row) + ", " + std::to_string(column) + ')'; }
		explicit operator Direction() const;
		inline double distance(const Position &other) const { return std::sqrt(std::pow(row - other.row, 2) + std::pow(column - other.column, 2)); }
		inline uint64_t taxiDistance(const Position &other) const { return static_cast<uint64_t>(std::abs(row - other.row) + std::abs(column - other.column)); }
		uint64_t maximumAxisDistance(const Position &) const;
		bool adjacent4(const Position &) const;
		bool operator<(const Position &) const;
		std::string simpleString() const { return std::to_string(row) + ',' + std::to_string(column); }
	};

	std::ostream & operator<<(std::ostream &, const Position &);

	void to_json(nlohmann::json &, const Position &);
	void from_json(const nlohmann::json &, Position &);
	Buffer & operator+=(Buffer &, const Position &);
	Buffer & operator<<(Buffer &, const Position &);
	Buffer & operator>>(Buffer &, Position &);

	/** Silly naming, perhaps. */
	struct Place {
		Position position;
		std::shared_ptr<Realm> realm;
		std::shared_ptr<Player> player;

		Place(Position position_, std::shared_ptr<Realm> realm_, std::shared_ptr<Player> player_ = {}):
			position(std::move(position_)), realm(std::move(realm_)), player(std::move(player_)) {}

		std::optional<TileID> get(Layer) const;
		std::optional<std::reference_wrapper<const Identifier>> getName(Layer) const;
		void set(Layer, TileID) const;
		void set(Layer, const Identifier &) const;
		bool isPathable() const;
		std::shared_ptr<TileEntity> getTileEntity() const;

		std::shared_ptr<Game> getGame() const;

		Place operator+(Direction) const;
		Place & operator+=(Direction);
		bool operator==(const Place &) const;
	};

	struct Vector3 {
		double x = 0.f;
		double y = 0.f;
		double z = 0.f;

		double magnitude() const;
		double magnitude2D() const;
		inline bool isGrounded() const { return z < 0.01; }

		bool operator==(const Vector3 &) const;

		Vector3 & operator+=(const Vector3 &);
		Vector3 & operator-=(const Vector3 &);
		Vector3 operator+(const Vector3 &) const;
		Vector3 operator-(const Vector3 &) const;
	};

	Buffer & operator+=(Buffer &, const Vector3 &);
	Buffer & operator<<(Buffer &, const Vector3 &);
	Buffer & operator>>(Buffer &, Vector3 &);

	struct Vector2f {
		double x = 0.f;
		double y = 0.f;
	};

	Buffer & operator+=(Buffer &, const Vector2f &);
	Buffer & operator<<(Buffer &, const Vector2f &);
	Buffer & operator>>(Buffer &, Vector2f &);

	struct Vector2i {
		int x = 0;
		int y = 0;
	};

	Buffer & operator+=(Buffer &, const Vector2i &);
	Buffer & operator<<(Buffer &, const Vector2i &);
	Buffer & operator>>(Buffer &, Vector2i &);
}

template <>
struct std::hash<Game3::Position> {
	size_t operator()(const Game3::Position &position) const noexcept {
		return (static_cast<size_t>(position.row) * 1298758219ul) ^ static_cast<size_t>(position.column);
	}
};

template <>
struct std::formatter<Game3::Position> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &position, auto &ctx) const {
		return std::format_to(ctx.out(), "({}, {})", position.row, position.column);
	}
};

template <>
struct std::formatter<Game3::Place> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &place, auto &ctx) const {
		if (place.realm) {
			if (place.player)
				return std::format_to(ctx.out(), "{}:{}:{}", place.position, place.realm->getID(), place.player->getUsername());
			return std::format_to(ctx.out(), "{}:{}:?", place.position, place.realm->getID());
		}

		if (place.player)
			return std::format_to(ctx.out(), "{}:?:{}", place.position, place.player->getUsername());
		return std::format_to(ctx.out(), "{}:?:?", place.position);
	}
};

template <>
struct std::formatter<Game3::Vector3> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &vector, auto &ctx) const {
		return std::format_to(ctx.out(), "({}, {}, {})", vector.x, vector.y, vector.z);
	}
};

template <>
struct std::formatter<Game3::Vector2f> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &vector, auto &ctx) const {
		return std::format_to(ctx.out(), "({}, {})", vector.x, vector.y);
	}
};

template <>
struct std::formatter<Game3::Vector2i> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const auto &vector, auto &ctx) const {
		return std::format_to(ctx.out(), "({}, {})", vector.x, vector.y);
	}
};
