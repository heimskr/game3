#include "game/Village.h"

namespace Game3 {
	Village::Village(const Position &position_):
		chunkPosition(position_),
		position(position_) {}

	Village::Village(ChunkPosition chunk_position, const Position &position_):
		chunkPosition(chunk_position),
		position(position_) {}
}
