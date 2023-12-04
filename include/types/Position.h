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

	struct Position {
		using value_type = Index;
		value_type row = 0;
		value_type column = 0;

		Position() = default;
		Position(value_type row_, value_type column_): row(row_), column(column_) {}
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
		Position & operator+=(Direction);
		Position & operator-=(Direction);
		Position & operator*=(Index);
		/** Has a bias for horizontal directions when the difference is diagonal. */
		Direction getFacing(const Position &other) const;
		explicit inline operator std::string() const { return '(' + std::to_string(row) + ", " + std::to_string(column) + ')'; }
		explicit operator Direction() const;
		inline double distance(const Position &other) const { return std::sqrt(std::pow(row - other.row, 2) + std::pow(column - other.column, 2)); }
		inline uint64_t taxiDistance(const Position &other) const { return static_cast<uint64_t>(std::abs(row - other.row) + std::abs(column - other.column)); }
		bool adjacent4(const Position &other) const;
		bool operator<(const Position &) const;
		std::string simpleString() const { return std::to_string(row) + ',' + std::to_string(column); }
	};

	void to_json(nlohmann::json &, const Position &);
	void from_json(const nlohmann::json &, Position &);
	std::ostream & operator<<(std::ostream &, const Position &);
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

		Game & getGame() const;

		Place operator+(Direction) const;
		Place & operator+=(Direction);
		bool operator==(const Place &) const;
	};

	std::ostream & operator<<(std::ostream &, const Place &);

	struct Vector3 {
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;
	};

	std::ostream & operator<<(std::ostream &, const Vector3 &);
	Buffer & operator+=(Buffer &, const Vector3 &);
	Buffer & operator<<(Buffer &, const Vector3 &);
	Buffer & operator>>(Buffer &, Vector3 &);

	struct Vector2 {
		float x = 0.f;
		float y = 0.f;
	};

	std::ostream & operator<<(std::ostream &, const Vector2 &);
	Buffer & operator+=(Buffer &, const Vector2 &);
	Buffer & operator<<(Buffer &, const Vector2 &);
	Buffer & operator>>(Buffer &, Vector2 &);
}

namespace std {
	template <>
	struct hash<Game3::Position> {
		size_t operator()(const Game3::Position &position) const noexcept {
			return (static_cast<size_t>(position.row) * 1298758219ul) ^ static_cast<size_t>(position.column);
		}
	};
}
