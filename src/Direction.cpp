#include "Direction.h"

std::ostream & operator<<(std::ostream &stream, Game3::Direction direction) {
	switch (direction) {
		case Game3::Direction::Up:    return stream << "up";
		case Game3::Direction::Down:  return stream << "down";
		case Game3::Direction::Left:  return stream << "left";
		case Game3::Direction::Right: return stream << "right";
		default: return stream << '?';
	}
}
