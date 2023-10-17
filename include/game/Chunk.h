#pragma once

#include "types/Types.h"
#include "threading/Lockable.h"

#include <vector>

namespace Game3 {
	template <typename T>
	using Chunk = Lockable<std::vector<T>>;

	using TileChunk  = Chunk<TileID>;
	using BiomeChunk = Chunk<BiomeType>;
	using PathChunk  = Chunk<uint8_t>;
}
