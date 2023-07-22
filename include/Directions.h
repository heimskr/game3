#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "Direction.h"
#include "Quadrant.h"

namespace Game3 {
	class Buffer;

	struct Directions {
		bool north  = false;
		bool east   = false;
		bool middle = false;
		bool south  = false;
		bool west   = false;

		Directions() = default;
		Directions(bool north_, bool east_, bool middle_, bool south_, bool west_);
		explicit Directions(uint8_t);

		explicit operator uint8_t() const;
		explicit operator std::string() const;

		inline bool toggleNorth()  { return north  = !north;  }
		inline bool toggleEast()   { return east   = !east;   }
		inline bool toggleMiddle() { return middle = !middle; }
		inline bool toggleSouth()  { return south  = !south;  }
		inline bool toggleWest()   { return west   = !west;   }

		bool & operator[](Quadrant);
		const bool & operator[](Quadrant) const;

		bool & operator[](Direction);
		const bool & operator[](Direction) const;

		bool toggle(Quadrant);
		bool toggle(Direction);

		std::vector<Direction> toVector() const;
		bool has(Direction) const;
		int8_t getMarchIndex() const;

		template <typename Fn>
		void iterate(const Fn &fn) {
			if (north)
				fn(Direction::Up);
			if (east)
				fn(Direction::Right);
			if (south)
				fn(Direction::Down);
			if (west)
				fn(Direction::Left);
		}

		static void iterate(const std::function<void(Direction)> &fn) {
			fn(Direction::Up);
			fn(Direction::Right);
			fn(Direction::Down);
			fn(Direction::Left);
		}

		/** Return false to break. */
		static void iterate(const std::function<bool(Direction)> &fn) {
			if (fn(Direction::Up))
				if (fn(Direction::Right))
					if (fn(Direction::Down))
						fn(Direction::Left);
		}

		/** (x, y) */
		template <typename T = int>
		inline std::pair<T, T> extractorOffsets() const {
			const uint8_t mask(*this);
			return {static_cast<T>(mask) % 8, static_cast<T>(mask) / 8};
		}
	};

	Buffer & operator+=(Buffer &, const Directions &);
	Buffer & operator<<(Buffer &, const Directions &);
	Buffer & operator>>(Buffer &, Directions &);
}
