#include "game/Village.h"

namespace Game3 {
	Village::Village(const Game &game, const Position &position_):
		Village(game, ChunkPosition(position_), position_) {}

	Village::Village(const Game &game, ChunkPosition chunk_position, const Position &position_):
		chunkPosition(chunk_position),
		position(position_),
		richness(Richness::makeRandom(game)) {}

	std::optional<double> Village::getRichness(const Identifier &identifier) {
		return richness[identifier];
	}
}
