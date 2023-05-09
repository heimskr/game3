#include "Direction.h"
#include "ThreadContext.h"

namespace Game3 {
	Direction remapDirection(Direction direction, uint16_t configuration) {
		switch (direction) {
			case Direction::Down:  return Direction((configuration >> 12) & 0xf);
			case Direction::Up:    return Direction((configuration >>  8) & 0xf);
			case Direction::Right: return Direction((configuration >>  4) & 0xf);
			case Direction::Left:  return Direction((configuration >>  0) & 0xf);
			default: return Direction::Up; // Could throw, but that might waste a few cycles.
		}
	}

	Direction randomDirection() {
		return static_cast<Direction>(std::uniform_int_distribution(0, 3)(threadContext.rng));
	}

	std::ostream & operator<<(std::ostream &stream, Direction direction) {
		switch (direction) {
			case Game3::Direction::Up:    return stream << "up";
			case Game3::Direction::Down:  return stream << "down";
			case Game3::Direction::Left:  return stream << "left";
			case Game3::Direction::Right: return stream << "right";
			default: return stream << '?';
		}
	}
}
