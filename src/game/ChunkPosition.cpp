#include "game/ChunkPosition.h"

namespace Game3 {
	ChunkPosition::operator std::string() const {
		return '(' + std::to_string(x) + ", " + std::to_string(y) + ')';
	}
}
