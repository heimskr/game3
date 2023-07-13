#include "Directions.h"
#include "pipes/PipeLoader.h"
#include "pipes/PipeNetwork.h"
#include "realm/Realm.h"
#include "tileentity/Pipe.h"

#include <atomic>

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

		if (auto by_chunk = realm.getTileEntities(chunk_position)) {
			auto lock = by_chunk->sharedLock();
			for (const TileEntityPtr &tile_entity: *by_chunk)
				if (auto pipe = tile_entity->cast<Pipe>(); pipe && !pipe->loaded)
					floodFill(pipe);
		}

		{
			auto unique_lock = busyChunks.uniqueLock();
			busyChunks.erase(chunk_position);
		}
	}

	void PipeLoader::floodFill(const std::shared_ptr<Pipe> &start) const {
		// The initial pipe needs to have not been loaded yet, and it can't already have a network.
		assert(!start->loaded);
		// If this assertion ever fails, something is horribly wrong.
		assert(!start->getNetwork());

		static std::atomic_size_t last_id = 0;

		auto network = std::make_shared<PipeNetwork>(++last_id);
		start->setNetwork(network);

		auto realm = start->getRealm();
		std::vector<std::shared_ptr<Pipe>> queue{start};

		while (!queue.empty()) {
			const std::shared_ptr<Pipe> pipe = std::move(queue.back());
			queue.pop_back();

			Directions &directions = pipe->getDirections();

			if (auto other_network = pipe->getNetwork()) {
				if (network != other_network)
					network->absorb(other_network);
			} else {
				pipe->setNetwork(network);
			}

			directions.iterate([&](Direction direction) {
				const Position neighbor_position = pipe->getPosition() + direction;
				if (TileEntityPtr base_neighbor = realm->tileEntityAt(neighbor_position)) {
					if (auto neighbor = base_neighbor->cast<Pipe>()) {
						// Check whether the connection is matched with a connection on the other pipe.
						if (!neighbor->loaded && neighbor->getDirections()[flipDirection(direction)])
							queue.push_back(neighbor);
					}
				}
			});
		}
	}
}
