#include "game/ServerGame.h"
#include "game/Village.h"

namespace Game3 {
	Village::Village(ServerGame &game, const Position &position_):
		Village(game, ChunkPosition(position_), position_) {}

	Village::Village(ServerGame &game, ChunkPosition chunk_position, const Position &position_):
		id(++game.lastVillageID),
		chunkPosition(chunk_position),
		position(position_),
		richness(Richness::makeRandom(game)) {}

	std::optional<double> Village::getRichness(const Identifier &identifier) {
		return richness[identifier];
	}
}
