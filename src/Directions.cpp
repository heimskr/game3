#include "Directions.h"
#include "MarchingSquares.h"

namespace Game3 {
	Directions::Directions(bool north_, bool east_, bool south_, bool west_, bool middle_):
		north(north_), east(east_), south(south_), west(west_), middle(middle_) {}

	Directions::Directions(uint8_t mask):
		north(mask & 1), east(mask & 2), south(mask & 4), west(mask & 8), middle(mask & 16) {}

	Directions::operator uint8_t() const {
		return north | (east << 1) | (south << 2) | (west << 3) | (middle << 4);
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
		const int sum = (north? 1 : 0) | (west? 2 : 0) | (east? 4 : 0) | (south? 8 : 0);
		return !middle || sum != 0? marchingArray4[sum] : 22;
	}
}
