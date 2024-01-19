#pragma once

#include <cstdint>
#include <vector>

namespace Game3 {
	class Realm;
	class ThreadPool;
	struct ChunkPosition;
	struct Position;

	struct VillageOptions {
		int width = 34;
		int height = 26;
		int padding = 2;

		VillageOptions() = default;
		VillageOptions(int width_, int height_, int padding_);
	};

	bool chunkValidForVillage(const ChunkPosition &, int realm_seed);

	std::vector<Position> getVillageCandidates(const Realm &realm, const ChunkPosition &, const VillageOptions &, ThreadPool &, std::optional<std::vector<Position>> starts = std::nullopt);
}
