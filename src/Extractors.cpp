#include "Extractors.h"

namespace Game3 {
	Extractors::Extractors(bool north_, bool east_, bool middle_, bool south_, bool west_):
		north(north_), east(east_), middle(middle_), south(south_), west(west_) {}

	Extractors::Extractors(uint8_t mask):
		north(mask & 1), east(mask & 2), middle(mask & 4), south(mask & 8), west(mask & 16) {}

	Extractors::operator uint8_t() const {
		return north | (east << 1) | (middle << 2) | (south << 3) | (west << 4);
	}

	Extractors::operator std::string() const {
		std::string out;
		if (north)  out += "N";
		if (east)   out += "E";
		if (middle) out += "M";
		if (south)  out += "S";
		if (west)   out += "W";
		if (out.empty())
			out = "_";
		return out;
	}

	bool Extractors::toggleQuadrant(Quadrant quadrant) {
		switch (quadrant) {
			case Quadrant::Top:    return toggleNorth();
			case Quadrant::Right:  return toggleEast();
			case Quadrant::Bottom: return toggleSouth();
			case Quadrant::Left:   return toggleWest();
			default:
				throw std::invalid_argument("Invalid quadrant: " + std::to_string(static_cast<uint8_t>(quadrant)));
		}
	}
}
