#pragma once

#include <nlohmann/json.hpp>

#include "Types.h"

namespace Game3 {
	struct Position {
		using value_type = Index;
		value_type row;
		value_type column;

		bool operator==(const Position &other) const { return row == other.row && column == other.column; }
		bool operator!=(const Position &other) const { return row != other.row || column != other.column; }
	};

	void to_json(nlohmann::json &, const Position &);
	void from_json(const nlohmann::json &, Position &);
}
