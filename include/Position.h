#pragma once

#include <nlohmann/json.hpp>

#include "Types.h"

namespace Game3 {
	struct Position {
		using value_type = Index;
		value_type row;
		value_type column;
	};

	void to_json(nlohmann::json &, const Position &);
	void from_json(const nlohmann::json &, Position &);
}
