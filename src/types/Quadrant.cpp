#include "types/Quadrant.h"

#include <array>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace Game3 {
	bool hasQuadrant(int8_t march_index, Quadrant quadrant) {
		static const std::unordered_set<int8_t> has_north {7, 9, 10, 14, 16, 17, 18, 19, 20, 26};
		static const std::unordered_set<int8_t> has_east  {0, 1,  4,  5, 12, 14, 15, 18, 19, 26};
		static const std::unordered_set<int8_t> has_south {0, 2,   3,  7,  9, 10, 12, 18, 19, 20};
		static const std::unordered_set<int8_t> has_west  {1, 2,  5,  6, 12, 15, 16, 19, 20, 26};

		switch (quadrant) {
			case Quadrant::Top:    return has_north.contains(march_index);
			case Quadrant::Bottom: return has_south.contains(march_index);
			case Quadrant::Right:  return has_east.contains(march_index);
			case Quadrant::Left:   return has_west.contains(march_index);
			default:
				return false;
		}
	}

	int8_t addQuadrant(int8_t march_index, Quadrant quadrant) {
		static const std::array<int8_t, 28> north_map {18, 26, 20,  7, 14, 26, 16,  7, 17,  7,  7, 17, 19, 17, 14, 26, 16, 17, 18, 19, 20, 17, 17, 17, 17, 27, 26, 17};
		static const std::array<int8_t, 28> east_map  { 0,  1, 12,  0,  4,  1,  1, 18,  4, 18, 18,  4, 12,  4, 14,  1, 26, 14, 18, 19, 19,  4,  4,  4,  4,  4, 26,  4};
		static const std::array<int8_t, 28> south_map { 0, 12,  2,  3,  0, 12,  2,  7,  3,  7,  7,  3, 12,  3, 18, 12, 20,  7, 18, 19, 20,  3,  3,  3,  3,  3, 19,  3};
		static const std::array<int8_t, 28> west_map  {12,  1,  2,  2,  1,  1,  6, 20,  6, 20, 20,  6, 12,  6, 26,  1, 16, 16, 19, 19, 20,  6,  6,  6,  6,  6, 26,  6};

		if (march_index < 0 || 27 < march_index)
			throw std::out_of_range("Invalid march index: " + std::to_string(march_index));

		switch (quadrant) {
			case Quadrant::Top:    return north_map[march_index];
			case Quadrant::Bottom: return south_map[march_index];
			case Quadrant::Right:  return east_map[march_index];
			case Quadrant::Left:   return west_map[march_index];
			default:
				throw std::invalid_argument("Invalid quadrant: " + std::to_string(static_cast<uint8_t>(quadrant)));
		}
	}

	int8_t removeQuadrant(int8_t march_index, Quadrant quadrant) {
		static const std::array<int8_t, 28> north_map {0,  1,  2,  3,  4,  1,  6,  3,  8,  3,  3,  8, 12,  8,  4,  1,  6, 22,  0, 12,  2,  8, 22,  8,  8,  8,  1, 8};
		static const std::array<int8_t, 28> east_map  {3,  6,  2,  3, 22,  6,  6,  7,  8,  7,  7,  8,  2,  8, 17,  1, 16, 17,  7, 20, 20,  8, 22,  8,  8,  8, 16, 8};
		static const std::array<int8_t, 28> south_map {4,  1,  6, 22,  4,  1,  6, 17,  8, 17, 17,  8,  1,  8, 14,  1, 16, 17, 14, 26, 16,  8, 22,  8,  8,  8, 26, 8};
		static const std::array<int8_t, 28> west_map  {0,  4,  3,  3,  4,  4, 22,  7,  8,  7,  7,  8,  0,  8, 14,  4, 17, 17, 18, 18,  7,  8, 22,  8,  8,  8, 14, 8};

		if (march_index < 0 || 27 < march_index)
			throw std::out_of_range("Invalid march index: " + std::to_string(march_index));

		switch (quadrant) {
			case Quadrant::Top:    return north_map[march_index];
			case Quadrant::Bottom: return south_map[march_index];
			case Quadrant::Right:  return east_map[march_index];
			case Quadrant::Left:   return west_map[march_index];
			default:
				throw std::invalid_argument("Invalid quadrant: " + std::to_string(static_cast<uint8_t>(quadrant)));
		}
	}

	int8_t toggleQuadrant(int8_t march_index, Quadrant quadrant) {
		return hasQuadrant(march_index, quadrant)? removeQuadrant(march_index, quadrant) : addQuadrant(march_index, quadrant);
	}

	Quadrant flipQuadrant(Quadrant quadrant) {
		switch (quadrant) {
			case Quadrant::Top:    return Quadrant::Bottom;
			case Quadrant::Right:  return Quadrant::Left;
			case Quadrant::Bottom: return Quadrant::Top;
			case Quadrant::Left:   return Quadrant::Right;
			default:
				return Quadrant::Invalid;
		}
	}

	Direction toDirection(Quadrant quadrant) {
		switch (quadrant) {
			case Quadrant::Top:    return Direction::Up;
			case Quadrant::Right:  return Direction::Right;
			case Quadrant::Bottom: return Direction::Down;
			case Quadrant::Left:   return Direction::Left;
			default:
				return Direction::Invalid;
		}
	}

	Quadrant toQuadrant(Direction direction) {
		switch (direction) {
			case Direction::Up:    return Quadrant::Top;
			case Direction::Right: return Quadrant::Right;
			case Direction::Down:  return Quadrant::Bottom;
			case Direction::Left:  return Quadrant::Left;
			default:
				return Quadrant::Invalid;
		}
	}

	Quadrant getQuadrant(double x, double y) {
		// Garbage variable names, but the original useful names weren't quite correct.
		const bool one = x <= y;
		const bool two = (1. - x) <= y;
		if (one)
			return two? Quadrant::Top : Quadrant::Right;
		return two? Quadrant::Left : Quadrant::Bottom;
	}

	std::string_view toString(Quadrant quadrant) {
		switch (quadrant) {
			case Quadrant::Top:    return "Top";
			case Quadrant::Right:  return "Right";
			case Quadrant::Bottom: return "Bottom";
			case Quadrant::Left:   return "Left";
			default:
				return "invalid";
		}
	}
}
