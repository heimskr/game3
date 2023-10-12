#include "Position.h"
#include "entity/Player.h"
#include "graphics/Tileset.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "util/Util.h"

namespace Game3 {
	Position::Position(std::string_view string) {
		const size_t comma = string.find(',');
		if (comma == std::string::npos)
			throw std::invalid_argument("Invalid position: \"" + std::string(string) + '"');
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
				ERROR("Direction " << static_cast<int>(direction) << " is invalid");
				std::terminate();
		}
	}

	Position & Position::operator+=(Direction direction) {
		switch (direction) {
			case Direction::Up:    return *this += Position(-1,  0);
			case Direction::Right: return *this += Position( 0,  1);
			case Direction::Down:  return *this += Position( 1,  0);
			case Direction::Left:  return *this += Position( 0, -1);
			default:
				ERROR("Direction " << static_cast<int>(direction) << " is invalid");
				std::terminate();
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

	Place Place::operator+(Direction direction) const {
		return {position + direction, realm, player};
	}

	Place & Place::operator+=(Direction direction) {
		position += direction;
		return *this;
	}

	bool Place::operator==(const Place &other) const {
		return this == &other || (position == other.position && realm == other.realm && player == other.player);
	}

	std::ostream & operator<<(std::ostream &os, const Place &place) {
		if (place.realm) {
			if (place.player)
				return os << place.position << ':' << place.realm->id << ':' << place.player->username;
			return os << place.position << ':' << place.realm->id << ":?";
		}

		if (place.player)
			return os << place.position << ":?:" << place.player->username;
		return os << place.position << ":?:?";
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

	template <>
	std::string Buffer::getType(const Offset &) {
		return std::string{'\x32'} + getType(float{});
	}

	std::ostream & operator<<(std::ostream &stream, const Offset &offset) {
		return stream << '(' << offset.x << ", " << offset.y << ", " << offset.z << ')';
	}

	Buffer & operator+=(Buffer &buffer, const Offset &offset) {
		return ((buffer.appendType(offset) += offset.x) += offset.y) += offset.z;
	}

	Buffer & operator<<(Buffer &buffer, const Offset &offset) {
		return buffer += offset;
	}

	Buffer & operator>>(Buffer &buffer, Offset &offset) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(offset))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected shortlist<f32, 3>)");
		}
		popBuffer(buffer, offset.x);
		popBuffer(buffer, offset.y);
		popBuffer(buffer, offset.z);
		return buffer;
	}
}
