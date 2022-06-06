#include "Position.h"

namespace Game3 {
	void to_json(nlohmann::json &json, const Position &position) {
		json[0] = position.row;
		json[1] = position.column;
	}

	void from_json(const nlohmann::json &json, Position &position) {
		position.row = json.at(0);
		position.column = json.at(1);
	}
}

std::ostream & operator<<(std::ostream &stream, const Game3::Position &position) {
	return stream << '(' << position.row << ", " << position.column << ')';
}
