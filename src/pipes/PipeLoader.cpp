#include "Directions.h"
#include "pipes/PipeLoader.h"
#include "realm/Realm.h"

namespace Game3 {
	PipeLoader::PipeLoader(Realm &realm_):
		realm(realm_) {}

	void PipeLoader::load(ChunkPosition chunk_position) {
		{
			auto shared_lock = busyChunks.sharedLock();
			if (busyChunks.contains(chunk_position))
				return;
		}

		{
			auto unique_lock = busyChunks.uniqueLock();
			if (busyChunks.contains(chunk_position))
				return;
			busyChunks.insert(chunk_position);
		}
	}
}
