#pragma once

#include "game/ChunkPosition.h"
#include "threading/Lockable.h"

#include <memory>
#include <unordered_set>

namespace Game3 {
	class Pipe;
	class Realm;

	class PipeLoader {
		private:
			Realm &realm;
			Lockable<std::unordered_set<ChunkPosition>> busyChunks;

			void floodFill(const std::shared_ptr<Pipe> &) const;

		public:
			PipeLoader(Realm &);

			void load(ChunkPosition);
	};
}
