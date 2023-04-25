#include "Position.h"
#include "Tileset.h"
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

	const Identifier & Place::getLayer1Name() const {
		return (*realm->tilemap1->tileset)[realm->getLayer1(position)];
	}

	const Identifier & Place::getLayer2Name() const {
		return (*realm->tilemap2->tileset)[realm->getLayer2(position)];
	}

	const Identifier & Place::getLayer3Name() const {
		return (*realm->tilemap3->tileset)[realm->getLayer3(position)];
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

	void Place::setLayer1(const Identifier &tilename) const {
		realm->setLayer1(position, (*realm->tilemap1->tileset)[tilename]);
	}

	void Place::setLayer2(const Identifier &tilename) const {
		realm->setLayer2(position, (*realm->tilemap2->tileset)[tilename]);
	}

	void Place::setLayer3(const Identifier &tilename) const {
		realm->setLayer3(position, (*realm->tilemap3->tileset)[tilename]);
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
