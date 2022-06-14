#pragma once

#include <cmath>
#include <functional>
#include <ostream>
#include <string>

#include <nlohmann/json.hpp>

#include "Types.h"

namespace Game3 {
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
		explicit inline operator bool() const { return 0 <= row && 0 <= column; }
		bool operator<(const Position &) const;
	};

	struct Location {
		Position position;
		RealmID realm;
	};

	void to_json(nlohmann::json &, const Position &);
	void to_json(nlohmann::json &, const Location &);
	void from_json(const nlohmann::json &, Position &);
	void from_json(const nlohmann::json &, Location &);
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
