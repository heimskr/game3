#include "entity/Player.h"
#include "fluid/Fluid.h"
#include "game/Game.h"
#include "game/TileProvider.h"
#include "graphics/Tileset.h"
#include "lib/JSON.h"
#include "net/Buffer.h"
#include "realm/Realm.h"
#include "types/ChunkPosition.h"
#include "types/Position.h"
#include "util/Util.h"

namespace Game3 {
	Position::Position(std::string_view string) {
		if (string.size() < 6 || string.front() != '(' || string.back() != ')')
			throw std::invalid_argument("Invalid Position string");

		string.remove_prefix(1);
		string.remove_suffix(1);

		size_t comma_size = 2;
		size_t comma = string.find(", ");

		if (comma == std::string_view::npos) {
			comma = string.find(',');
			comma_size = 1;
			if (comma == std::string_view::npos)
				throw std::invalid_argument("Invalid Position string");
		}

		row = parseNumber<IntType>(string.substr(0, comma));
		column = parseNumber<IntType>(string.substr(comma + comma_size));
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
				throw std::invalid_argument("Direction " + std::to_string(static_cast<int>(direction)) + " is invalid");
		}
	}

	Position Position::operator-(Direction direction) const {
		switch (direction) {
			case Direction::Up:    return *this - Position(-1,  0);
			case Direction::Right: return *this - Position( 0,  1);
			case Direction::Down:  return *this - Position( 1,  0);
			case Direction::Left:  return *this - Position( 0, -1);
			default:
				throw std::invalid_argument("Direction " + std::to_string(static_cast<int>(direction)) + " is invalid");
		}
	}

	Position Position::operator*(Index factor) const {
		return {row * factor, column * factor};
	}

	Position Position::operator/(Index divisor) const {
		return {row / divisor, column / divisor};
	}

	Position Position::operator*(double factor) const {
		return {IntType(row * factor), IntType(column * factor)};
	}

	Position Position::operator/(double divisor) const {
		return {IntType(row / divisor), IntType(column / divisor)};
	}

	Position & Position::operator+=(Direction direction) {
		switch (direction) {
			case Direction::Up:    return *this += Position(-1,  0);
			case Direction::Right: return *this += Position( 0,  1);
			case Direction::Down:  return *this += Position( 1,  0);
			case Direction::Left:  return *this += Position( 0, -1);
			default:
				throw std::invalid_argument("Direction " + std::to_string(static_cast<int>(direction)) + " is invalid");
		}
	}

	Position & Position::operator-=(Direction direction) {
		switch (direction) {
			case Direction::Up:    return *this -= Position(-1,  0);
			case Direction::Right: return *this -= Position( 0,  1);
			case Direction::Down:  return *this -= Position( 1,  0);
			case Direction::Left:  return *this -= Position( 0, -1);
			default:
				throw std::invalid_argument("Direction " + std::to_string(static_cast<int>(direction)) + " is invalid");
		}
	}

	Position & Position::operator*=(Index factor) {
		row *= factor;
		column *= factor;
		return *this;
	}

	Position & Position::operator/=(Index divisor) {
		row /= divisor;
		column /= divisor;
		return *this;
	}

	Position & Position::operator*=(double factor) {
		row *= factor;
		column *= factor;
		return *this;
	}

	Position & Position::operator/=(double divisor) {
		row /= divisor;
		column /= divisor;
		return *this;
	}

	ChunkPosition Position::getChunk() const {
		return {TileProvider::divide<ChunkPosition::IntType>(column), TileProvider::divide<ChunkPosition::IntType>(row)};
	}

	Direction Position::getFacing(const Position &other) const {
		Position difference = other - *this;

		if (difference.row == 0 && difference.column == 0)
			return Direction::Invalid;

		if (difference.column < 0) {
			if (std::abs(difference.column) >= std::abs(difference.row))
				return Direction::Left;
		} else if (difference.column > 0) {
			if (std::abs(difference.column) >= std::abs(difference.row))
				return Direction::Right;
		}

		return difference.row < 0? Direction::Up : Direction::Down;
	}

	Position::operator Direction() const {
		// Diagonals and zero are invalid.
		if ((row == 0) == (column == 0))
			return Direction::Invalid;

		if (row == 0)
			return column < 0? Direction::Left : Direction::Right;

		return row < 0? Direction::Up : Direction::Down;
	}

	uint64_t Position::maximumAxisDistance(const Position &other) const {
		return uint64_t(std::max(std::abs(row - other.row), std::abs(column - other.column)));
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

	std::optional<FluidTile> Place::getFluid() const {
		return realm->tryFluid(position);
	}

	std::optional<std::reference_wrapper<const Identifier>> Place::getName(Layer layer) const {
		if (auto tile = realm->tryTile(layer, position)) {
			return realm->getTileset()[*tile];
		}
		return std::nullopt;
	}

	void Place::set(Layer layer, TileID tile) const {
		realm->setTile(layer, position, tile);
	}

	void Place::set(Layer layer, const Identifier &tilename) const {
		realm->setTile(layer, position, tilename);
	}

	void Place::setFluid(FluidTile fluid_tile) const {
		realm->setFluid(position, fluid_tile);
	}

	bool Place::isPathable() const {
		return realm->isPathable(position);
	}

	std::shared_ptr<Tile> Place::getTile(Layer layer) const {
		if (std::optional<TileID> tile_id = get(layer)) {
			return realm->getGame()->getTile(realm->getTileset()[*tile_id]);
		}

		return {};
	}

	std::shared_ptr<TileEntity> Place::getTileEntity() const {
		return realm->tileEntityAt(position);
	}

	Place Place::withPosition(Position new_position) const {
		return Place{new_position, realm, player};
	}

	std::shared_ptr<Game> Place::getGame() const {
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

	std::ostream & operator<<(std::ostream &os, const Position &position) {
		return os << '(' << position.row << ", " << position.column << ')';
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const Position &position) {
		auto &array = json.emplace_array();
		array.emplace_back(position.row);
		array.emplace_back(position.column);
	}

	Position tag_invoke(boost::json::value_to_tag<Position>, const boost::json::value &json) {
		auto row = getNumber<Position::IntType>(json.at(0));
		auto column = getNumber<Position::IntType>(json.at(1));
		return {row, column};
	}

	template <>
	std::string Buffer::getType(const Position &, bool) {
		return std::string{'\xe9'};
	}

	Buffer & operator+=(Buffer &buffer, const Position &position) {
		buffer.appendType(position, false);
		if constexpr (Position::ROW_FIRST) {
			return (buffer += position.row) += position.column;
		} else {
			return (buffer += position.column) += position.row;
		}
	}

	Buffer & operator<<(Buffer &buffer, const Position &position) {
		return buffer += position;
	}

	Buffer & operator>>(Buffer &buffer, Position &position) {
		const auto type = buffer.popType();
		if (!Buffer::typesMatch(type, buffer.getType(position, false))) {
			buffer.debug();
			throw std::invalid_argument("Invalid type (" + hexString(type, true) + ") in buffer (expected e9 for Position)");
		}
		if constexpr (Position::ROW_FIRST) {
			position.row = popBuffer<Position::IntType>(buffer);
			position.column = popBuffer<Position::IntType>(buffer);
		} else {
			position.column = popBuffer<Position::IntType>(buffer);
			position.row = popBuffer<Position::IntType>(buffer);
		}
		return buffer;
	}
}
