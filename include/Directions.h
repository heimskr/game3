#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "Direction.h"
#include "Quadrant.h"

namespace Game3 {
	struct Directions {
		bool north  = false;
		bool east   = false;
		bool south  = false;
		bool west   = false;
		bool middle = false;

		Directions() = default;
		Directions(bool north_, bool east_, bool south_, bool west_, bool middle_);
		explicit Directions(uint8_t);

		explicit operator uint8_t() const;
		explicit operator std::string() const;

		inline bool toggleNorth()  { return north  = !north;  }
		inline bool toggleEast()   { return east   = !east;   }
		inline bool toggleSouth()  { return south  = !south;  }
		inline bool toggleWest()   { return west   = !west;   }
		inline bool toggleMiddle() { return middle = !middle; }

		bool toggle(Quadrant);
		bool toggle(Direction);
		std::vector<Direction> toVector() const;
		bool has(Direction) const;
		int8_t getMarchIndex() const;
	};
}
