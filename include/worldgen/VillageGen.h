#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace Game3 {
	class Realm;
	class ThreadPool;
	struct ChunkPosition;
	struct ChunkRange;
	struct Position;

	struct VillageOptions {
		int width   = 34;
		int height  = 26;
		int padding = 2;

		VillageOptions() = default;
		VillageOptions(int width_, int height_, int padding_);
	};

	std::optional<Position> getVillagePosition(const Realm &realm, const ChunkRange &, const VillageOptions &, ThreadPool &);
	std::optional<Position> getVillagePosition(const Realm &realm, const ChunkPosition &, const VillageOptions &, ThreadPool &, std::optional<std::vector<Position>> starts = std::nullopt);
	bool chunkValidForVillage(const ChunkPosition &, int realm_seed);
	std::vector<Position> getVillageCandidates(const Realm &realm, const ChunkPosition &, const VillageOptions &, ThreadPool &, std::optional<std::vector<Position>> starts = std::nullopt);
}
