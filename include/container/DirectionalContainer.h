#pragma once

#include "Direction.h"

namespace Game3 {
	template <typename T>
	struct DirectionalContainer {
		T north {};
		T east  {};
		T south {};
		T west  {};

		inline T & operator[](Direction direction) {
			switch (direction) {
				case Direction::Up:    return north;
				case Direction::Right: return east;
				case Direction::Down:  return south;
				case Direction::Left:  return west;
				default:
					throw std::out_of_range("Invalid direction: " + std::to_string(static_cast<uint8_t>(direction)));
			}
		}

		const T & operator[](Direction direction) const {
			switch (direction) {
				case Direction::Up:    return north;
				case Direction::Right: return east;
				case Direction::Down:  return south;
				case Direction::Left:  return west;
				default:
					throw std::out_of_range("Invalid direction: " + std::to_string(static_cast<uint8_t>(direction)));
			}
		}
	};
}
