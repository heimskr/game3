#pragma once

#include "types/VillageOptions.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace Game3 {
	class Realm;
	class ThreadPool;
	struct ChunkPosition;
	struct ChunkRange;
	struct Position;

	std::optional<Position> tryGenerateVillage(const std::shared_ptr<Realm> &realm, const ChunkPosition &, ThreadPool &);
	std::optional<Position> getVillagePosition(const Realm &realm, const ChunkRange &, const VillageOptions &, ThreadPool &);
	std::optional<Position> getVillagePosition(const Realm &realm, const ChunkPosition &, const VillageOptions &, ThreadPool &, std::optional<std::vector<Position>> starts = std::nullopt);
	bool chunkValidForVillage(const ChunkPosition &, int realm_seed);
	std::vector<Position> getVillageCandidates(const Realm &realm, const ChunkPosition &, const VillageOptions &, ThreadPool &, std::optional<std::vector<Position>> starts = std::nullopt);
}
