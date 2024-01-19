#pragma once

#include "data/Richness.h"
#include "types/ChunkPosition.h"

namespace Game3 {
	class Game;

	class Village {
		public:
			Village(const Game &, ChunkPosition, const Position &);
			Village(const Game &, const Position &);

			inline auto getChunkPosition() const { return chunkPosition; }
			inline auto getPosition() const { return position; }
			std::optional<double> getRichness(const Identifier &);

		private:
			ChunkPosition chunkPosition;
			Position position;
			Richness richness;
	};
}
