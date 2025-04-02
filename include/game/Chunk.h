#pragma once

#include "types/Types.h"
#include "threading/Lockable.h"

#include <vector>

namespace Game3 {
	template <typename T>
	struct Chunk: Lockable<std::vector<T>> {
		using Lockable<std::vector<T>>::Lockable;
		uint64_t updateCounter = 0;
	};

	using TileChunk  = Chunk<TileID>;
	using BiomeChunk = Chunk<BiomeType>;
	using PathChunk  = Chunk<uint8_t>;
}
