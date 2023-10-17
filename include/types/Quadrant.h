#pragma once

#include <cstdint>
#include <ostream>
#include <string>

#include "types/Direction.h"

namespace Game3 {
	/** 4 isoceles triangles in a square, with two points at adjacent corners and another point at the center of the square. */
	enum class Quadrant: uint8_t {Invalid = 0, Top, Right, Bottom, Left};

	bool hasQuadrant(int8_t march_index, Quadrant);
	int8_t addQuadrant(int8_t march_index, Quadrant);
	int8_t removeQuadrant(int8_t march_index, Quadrant);
	int8_t toggleQuadrant(int8_t march_index, Quadrant);

	Quadrant flipQuadrant(Quadrant);

	Direction toDirection(Quadrant);
	Quadrant toQuadrant(Direction);

	/** x and y are expected to be in the range [0, 1). */
	Quadrant getQuadrant(float x, float y);

	std::string_view toString(Quadrant);
	std::ostream & operator<<(std::ostream &, Quadrant);
}
