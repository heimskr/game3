#include "Position.h"

namespace Game3 {
	void to_json(nlohmann::json &json, const Position &position) {
		json[0] = position.row;
		json[1] = position.column;
	}

	void to_json(nlohmann::json &json, const Location &location) {
		json[0] = location.position.row;
		json[1] = location.position.column;
		json[2] = location.realm;
	}

	void from_json(const nlohmann::json &json, Position &position) {
		position.row = json.at(0);
		position.column = json.at(1);
	}

	void from_json(const nlohmann::json &json, Location &location) {
		location.position.row = json.at(0);
		location.position.column = json.at(1);
		location.realm = json.at(2);
	}
}

std::ostream & operator<<(std::ostream &stream, const Game3::Position &position) {
	return stream << '(' << position.row << ", " << position.column << ')';
}
