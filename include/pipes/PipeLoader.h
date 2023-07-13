#pragma once

#include "game/ChunkPosition.h"
#include "threading/Lockable.h"

#include <memory>
#include <unordered_set>

namespace Game3 {
	class Realm;

	class PipeLoader {
		private:
			Realm &realm;
			Lockable<std::unordered_set<ChunkPosition>> busyChunks;

		public:
			PipeLoader(Realm &);

			void load(ChunkPosition);
	};
}
