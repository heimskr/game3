#pragma once

#include "data/Richness.h"
#include "types/ChunkPosition.h"

namespace Game3 {
	class ServerGame;

	class Village {
		public:
			Village(ServerGame &, ChunkPosition, const Position &);
			Village(ServerGame &, const Position &);

			inline auto getChunkPosition() const { return chunkPosition; }
			inline auto getPosition() const { return position; }
			std::optional<double> getRichness(const Identifier &);

		private:
			size_t id;
			ChunkPosition chunkPosition;
			Position position;
			Richness richness;
	};
}
