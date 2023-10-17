#include "types/Direction.h"
#include "threading/ThreadContext.h"
#include "net/Buffer.h"

namespace Game3 {
	Direction remapDirection(Direction direction, uint16_t configuration) {
		switch (direction) {
			case Direction::Down:  return Direction((configuration >> 12) & 0xf);
			case Direction::Up:    return Direction((configuration >>  8) & 0xf);
			case Direction::Right: return Direction((configuration >>  4) & 0xf);
			case Direction::Left:  return Direction((configuration >>  0) & 0xf);
			default:
				// Could throw, but that might waste a few cycles.
				return Direction::Invalid;
		}
	}

	Direction rotateClockwise(Direction direction) {
		switch (direction) {
			case Direction::Up:    return Direction::Right;
			case Direction::Right: return Direction::Down;
			case Direction::Down:  return Direction::Left;
			case Direction::Left:  return Direction::Up;
			default:
				return Direction::Invalid;
		}
	}

	Direction rotateCounterClockwise(Direction direction) {
		switch (direction) {
			case Direction::Up:    return Direction::Left;
			case Direction::Right: return Direction::Up;
			case Direction::Down:  return Direction::Right;
			case Direction::Left:  return Direction::Down;
			default:
				return Direction::Invalid;
		}
	}

	Direction flipDirection(Direction direction) {
		switch (direction) {
			case Direction::Up:    return Direction::Down;
			case Direction::Right: return Direction::Left;
			case Direction::Down:  return Direction::Up;
			case Direction::Left:  return Direction::Right;
			default:
				return Direction::Invalid;
		}
	}

	Direction randomDirection() {
		return static_cast<Direction>(std::uniform_int_distribution(0, 3)(threadContext.rng));
	}

	std::string toString(Direction direction) {
		switch (direction) {
			case Game3::Direction::Up:    return "up";
			case Game3::Direction::Down:  return "down";
			case Game3::Direction::Left:  return "left";
			case Game3::Direction::Right: return "right";
			default:
				return "?";
		}
	}

	std::ostream & operator<<(std::ostream &os, Direction direction) {
		return os << toString(direction);
	}

	Buffer & operator+=(Buffer &buffer, Direction direction) {
		return buffer += static_cast<uint8_t>(direction);
	}

	Buffer & operator<<(Buffer &buffer, Direction direction) {
		return buffer << static_cast<uint8_t>(direction);
	}

	Buffer & operator>>(Buffer &buffer, Direction &direction) {
		direction = static_cast<Direction>(buffer.take<uint8_t>());
		return buffer;
	}
}
