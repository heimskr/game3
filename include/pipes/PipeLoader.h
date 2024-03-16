#pragma once

#include "types/ChunkPosition.h"
#include "threading/Lockable.h"

#include <atomic>
#include <memory>
#include <unordered_set>

namespace Game3 {
	class Pipe;
	class Realm;

	class PipeLoader {
		private:
			Lockable<std::unordered_set<ChunkPosition>> busyChunks;
			std::atomic_size_t lastID = 0;

		public:
			PipeLoader() = default;

			void load(Realm &, ChunkPosition);
			void floodFill(Substance, const std::shared_ptr<Pipe> &);
			size_t newID() { return ++lastID; }
	};
}
