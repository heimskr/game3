#include "Directions.h"
#include "MarchingSquares.h"
#include "net/Buffer.h"

namespace Game3 {
	Directions::Directions(bool north_, bool east_, bool middle_, bool south_, bool west_):
		north(north_), east(east_), middle(middle_), south(south_), west(west_) {}

	Directions::Directions(uint8_t mask):
		north(mask & 1), east(mask & 2), middle(mask & 4), south(mask & 8), west(mask & 16) {}

	Directions::operator uint8_t() const {
		return north | (east << 1) | (middle << 2) | (south << 3) | (west << 4);
	}

	Directions::operator std::string() const {
		std::string out;
		if (north)  out += "N";
		if (east)   out += "E";
		if (south)  out += "S";
		if (west)   out += "W";
		if (middle) out += "M";
		if (out.empty())
			out = "_";
		return out;
	}

	bool & Directions::operator[](Quadrant quadrant) {
		switch (quadrant) {
			case Quadrant::Top:    return north;
			case Quadrant::Right:  return east;
			case Quadrant::Bottom: return south;
			case Quadrant::Left:   return west;
			default:
				throw std::invalid_argument("Invalid quadrant: " + std::to_string(static_cast<uint8_t>(quadrant)));
		}
	}

	const bool & Directions::operator[](Quadrant quadrant) const {
		switch (quadrant) {
			case Quadrant::Top:    return north;
			case Quadrant::Right:  return east;
			case Quadrant::Bottom: return south;
			case Quadrant::Left:   return west;
			default:
				throw std::invalid_argument("Invalid quadrant: " + std::to_string(static_cast<uint8_t>(quadrant)));
		}
	}

	bool & Directions::operator[](Direction direction) {
		switch (direction) {
			case Direction::Up:    return north;
			case Direction::Right: return east;
			case Direction::Down:  return south;
			case Direction::Left:  return west;
			default:
				throw std::invalid_argument("Invalid direction: " + std::to_string(static_cast<uint8_t>(direction)));
		}
	}

	const bool & Directions::operator[](Direction direction) const {
		switch (direction) {
			case Direction::Up:    return north;
			case Direction::Right: return east;
			case Direction::Down:  return south;
			case Direction::Left:  return west;
			default:
				throw std::invalid_argument("Invalid direction: " + std::to_string(static_cast<uint8_t>(direction)));
		}
	}

	bool Directions::toggle(Quadrant quadrant) {
		switch (quadrant) {
			case Quadrant::Top:    return toggleNorth();
			case Quadrant::Right:  return toggleEast();
			case Quadrant::Bottom: return toggleSouth();
			case Quadrant::Left:   return toggleWest();
			default:
				throw std::invalid_argument("Invalid quadrant: " + std::to_string(static_cast<uint8_t>(quadrant)));
		}
	}

	bool Directions::toggle(Direction direction) {
		switch (direction) {
			case Direction::Up:    return toggleNorth();
			case Direction::Right: return toggleEast();
			case Direction::Down:  return toggleSouth();
			case Direction::Left:  return toggleWest();
			default:
				throw std::invalid_argument("Invalid direction: " + std::to_string(static_cast<uint8_t>(direction)));
		}
	}

	std::vector<Direction> Directions::toVector() const {
		std::vector<Direction> out;
		if (north) out.push_back(Direction::Up);
		if (east)  out.push_back(Direction::Right);
		if (south) out.push_back(Direction::Down);
		if (west)  out.push_back(Direction::Left);
		return out;
	}

	bool Directions::has(Direction direction) const {
		switch (direction) {
			case Direction::Up:    return north;
			case Direction::Right: return east;
			case Direction::Down:  return south;
			case Direction::Left:  return west;
			default:
				return false;
		}
	}

	int8_t Directions::getMarchIndex() const {
		return (north? 1 : 0) | (west? 2 : 0) | (east? 4 : 0) | (south? 8 : 0);
	}

	Buffer & operator+=(Buffer &buffer, const Directions &directions) {
		return ((((buffer += directions.north) += directions.east) += directions.south) += directions.west) += directions.middle;
	}

	Buffer & operator<<(Buffer &buffer, const Directions &directions) {
		return buffer << directions.north << directions.east << directions.south << directions.west << directions.middle;
	}

	Buffer & operator>>(Buffer &buffer, Directions &directions) {
		return buffer >> directions.north >> directions.east >> directions.south >> directions.west >> directions.middle;
	}
}
