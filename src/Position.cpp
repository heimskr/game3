#include "Position.h"
#include "Tileset.h"
#include "realm/Realm.h"
#include "util/Util.h"

namespace Game3 {
	Position::Position(std::string_view string) {
		const auto comma = string.find(',');
		row = parseLong(string.substr(0, comma));
		column = parseLong(string.substr(comma + 1));
	}

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

	TileID Place::get(Layer layer) const {
		return realm->getTile(layer, position);
	}

	const Identifier & Place::getName(Layer layer) const {
		return realm->getTileset()[realm->getTile(layer, position)];
	}

	void Place::set(Layer layer, TileID tile) const {
		realm->setTile(layer, position, tile);
	}

	void Place::set(Layer layer, const Identifier &tilename) const {
		realm->setTile(layer, position, tilename);
	}

	Game & Place::getGame() {
		return realm->getGame();
	}

	const Game & Place::getGame() const {
		return realm->getGame();
	}

	bool Place::operator==(const Place &other) const {
		return this == &other || (position == other.position && realm == other.realm && player == other.player);
	}

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
