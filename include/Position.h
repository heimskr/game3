#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <ostream>
#include <string>

#include <nlohmann/json.hpp>

#include "Types.h"

namespace Game3 {
	class Game;
	class Realm;

	struct Position {
		using value_type = Index;
		value_type row = 0;
		value_type column = 0;

		Position() = default;
		Position(value_type row_, value_type column_): row(row_), column(column_) {}
		inline bool operator==(const Position &other) const { return row == other.row && column == other.column; }
		inline bool operator!=(const Position &other) const { return row != other.row || column != other.column; }
		inline Position operator+(const Position &other) const { return {row + other.row, column + other.column}; }
		inline Position operator-(const Position &other) const { return {row - other.row, column - other.column}; }
		inline Position & operator+=(const Position &other) { row += other.row; column += other.column; return *this; }
		inline Position & operator-=(const Position &other) { row -= other.row; column -= other.column; return *this; }
		inline operator std::string() const { return '(' + std::to_string(row) + ", " + std::to_string(column) + ')'; }
		inline double distance(const Position &other) const { return std::sqrt(std::pow(row - other.row, 2) + std::pow(column - other.column, 2)); }
		bool adjacent4(const Position &other) const;
		explicit inline operator bool() const { return 0 <= row && 0 <= column; }
		bool operator<(const Position &) const;
	};

	/** Silly naming, but easier than PointedLocation or LocationButWithARealmPtrInsteadOfARealmID. */
	struct Place {
		Position position;
		std::shared_ptr<Realm> realm;
		std::shared_ptr<Player> player;

		Place(Position position_, std::shared_ptr<Realm> realm_, std::shared_ptr<Player> player_):
			position(std::move(position_)), realm(std::move(realm_)), player(std::move(player_)) {}

		TileID getLayer1() const;
		TileID getLayer2() const;
		TileID getLayer3() const;
		const Identifier & getLayer1Name() const;
		const Identifier & getLayer2Name() const;
		const Identifier & getLayer3Name() const;
		void setLayer1(TileID) const;
		void setLayer2(TileID) const;
		void setLayer3(TileID) const;
		void setLayer1(const Identifier &) const;
		void setLayer2(const Identifier &) const;
		void setLayer3(const Identifier &) const;

		Game & getGame();
		const Game & getGame() const;

		bool operator==(const Place &) const;
	};

	void to_json(nlohmann::json &, const Position &);
	void from_json(const nlohmann::json &, Position &);
}

std::ostream & operator<<(std::ostream &, const Game3::Position &);

namespace std {
	template <>
	struct hash<Game3::Position> {
		size_t operator()(const Game3::Position &position) const noexcept {
			return std::hash<Game3::Position::value_type>()(position.row ^ (position.column << 16));
		}
	};
}
