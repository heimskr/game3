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
			Lockable<std::unordered_set<ChunkPosition>> busyChunks;

		public:
			PipeLoader() = default;

			void load(Realm &, ChunkPosition);
			void floodFill(PipeType, const std::shared_ptr<Pipe> &) const;
	};
}
