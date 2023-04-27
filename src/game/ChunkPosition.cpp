#include "game/ChunkPosition.h"

namespace Game3 {
	bool ChunkPosition::operator==(const ChunkPosition &other) const {
		return this == &other || (x == other.x && y == other.y);
	}

	ChunkPosition::operator std::string() const {
		return '(' + std::to_string(x) + ", " + std::to_string(y) + ')';
	}
}
