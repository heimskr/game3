#pragma once

#include <functional>
#include <ostream>

#include <nlohmann/json.hpp>

#include "Types.h"

namespace Game3 {
	struct Position {
		using value_type = Index;
		value_type row;
		value_type column;

		inline bool operator==(const Position &other) const { return row == other.row && column == other.column; }
		inline bool operator!=(const Position &other) const { return row != other.row || column != other.column; }
		inline Position operator+(const Position &other) const { return {row + other.row, column + other.column}; }
		inline Position & operator+=(const Position &other) { row += other.row; column += other.column; return *this; }
		explicit inline operator bool() const { return row != 0 || column != 0; }
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
