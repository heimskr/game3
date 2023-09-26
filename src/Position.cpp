#include "Position.h"
#include "graphics/Tileset.h"
#include "net/Buffer.h"
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

	Position Position::operator+(Direction direction) const {
		switch (direction) {
			case Direction::Up:    return *this + Position(-1,  0);
			case Direction::Right: return *this + Position( 0,  1);
			case Direction::Down:  return *this + Position( 1,  0);
			case Direction::Left:  return *this + Position( 0, -1);
			default:
				assert(!"Direction is valid");
				return *this;
		}
	}

	Position & Position::operator+=(Direction direction) {
		switch (direction) {
			case Direction::Up:    return *this += Position(-1,  0);
			case Direction::Right: return *this += Position( 0,  1);
			case Direction::Down:  return *this += Position( 1,  0);
			case Direction::Left:  return *this += Position( 0, -1);
			default:
				assert(!"Direction is valid");
				return *this;
		}
	}

	Position::operator Direction() const {
		// Diagonals and zero are invalid.
		if ((row == 0) == (column == 0))
			return Direction::Invalid;

		if (row == 0)
			return column < 0? Direction::Left : Direction::Right;

		return row < 0? Direction::Up : Direction::Down;
	}

	bool Position::operator<(const Position &other) const {
		if (row < other.row)
			return true;
		if (other.row < row)
			return false;
		return column < other.column;
	}

	std::optional<TileID> Place::get(Layer layer) const {
		return realm->tryTile(layer, position);
	}

	std::optional<std::reference_wrapper<const Identifier>> Place::getName(Layer layer) const {
		if (auto tile = realm->tryTile(layer, position))
			return realm->getTileset()[*tile];
		return std::nullopt;
	}

	void Place::set(Layer layer, TileID tile) const {
		realm->setTile(layer, position, tile);
	}

	void Place::set(Layer layer, const Identifier &tilename) const {
		realm->setTile(layer, position, tilename);
	}

	bool Place::isPathable() const {
		return realm->isPathable(position);
	}

	Game & Place::getGame() const {
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

	std::ostream & operator<<(std::ostream &stream, const Position &position) {
		return stream << '(' << position.row << ", " << position.column << ')';
	}

	Buffer & operator+=(Buffer &buffer, const Position &position) {
		return (buffer += position.row) += position.column;
	}

	Buffer & operator<<(Buffer &buffer, const Position &position) {
		return buffer << position.row << position.column;
	}

	Buffer & operator>>(Buffer &buffer, Position &position) {
		return buffer >> position.row >> position.column;
	}

	std::ostream & operator<<(std::ostream &stream, const Offset &offset) {
		return stream << '(' << offset.x << ", " << offset.y << ", " << offset.z << ')';
	}

	Buffer & operator+=(Buffer &buffer, const Offset &offset) {
		return ((buffer += offset.x) += offset.y) += offset.z;
	}

	Buffer & operator<<(Buffer &buffer, const Offset &offset) {
		return buffer << offset.x << offset.y << offset.z;
	}

	Buffer & operator>>(Buffer &buffer, Offset &offset) {
		return buffer >> offset.x >> offset.y >> offset.z;
	}
}
