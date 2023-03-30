#include "Position.h"
#include "realm/Realm.h"

namespace Game3 {
	bool Position::adjacent4(const Position &other) const {
		const auto diff = *this - other;
		return (diff.row == 0 && (diff.column == 1 || diff.column == -1)) || (diff.column == 0 && (diff.row == 1 || diff.row == -1));
	}

	bool Position::operator<(const Position &other) const {
		if (row < other.row)
			return true;
		if (other.row < row)
			return false;
		return column < other.column;
	}


	TileID Place::getLayer1() const {
		return realm->getLayer1(position);
	}

	TileID Place::getLayer2() const {
		return realm->getLayer2(position);
	}

	TileID Place::getLayer3() const {
		return realm->getLayer3(position);
	}

	void Place::setLayer1(TileID tile) const {
		realm->setLayer1(position, tile);
	}

	void Place::setLayer2(TileID tile) const {
		realm->setLayer2(position, tile);
	}

	void Place::setLayer3(TileID tile) const {
		realm->setLayer3(position, tile);
	}

	bool Place::operator==(const Place &other) const {
		return this == &other || (position == other.position && realm == other.realm && player == other.player);
	}

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
