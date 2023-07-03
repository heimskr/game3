#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include "Quadrant.h"

namespace Game3 {
	struct Extractors {
		bool north  = false;
		bool east   = false;
		bool middle = false;
		bool south  = false;
		bool west   = false;

		Extractors() = default;
		Extractors(bool north_, bool east_, bool middle_, bool south_, bool west_);
		explicit Extractors(uint8_t);

		explicit operator uint8_t() const;
		explicit operator std::string() const;

		/** (x, y) */
		template <typename T = int>
		inline std::pair<T, T> getOffsets() const {
			const uint8_t mask(*this);
			return {mask % 8, mask / 8};
		}

		inline bool toggleNorth()  { return north  = !north;  }
		inline bool toggleEast()   { return east   = !east;   }
		inline bool toggleMiddle() { return middle = !middle; }
		inline bool toggleSouth()  { return south  = !south;  }
		inline bool toggleWest()   { return west   = !west;   }

		bool toggleQuadrant(Quadrant);
	};
}
