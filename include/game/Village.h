#pragma once

#include "types/ChunkPosition.h"

namespace Game3 {
	class Village {
		public:
			Village(ChunkPosition, const Position &);
			Village(const Position &);

			inline auto getChunkPosition() const { return chunkPosition; }
			inline auto getPosition() const { return position; }

		private:
			ChunkPosition chunkPosition;
			Position position;
	};
}
