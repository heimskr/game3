#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace Game3 {
	struct ChunkPosition {
		int32_t x = 0;
		int32_t y = 0;

		bool operator==(const ChunkPosition &) const;
		explicit operator std::string() const;
	};
}

namespace std {
	template <>
	struct hash<Game3::ChunkPosition> {
		size_t operator()(const Game3::ChunkPosition &chunk_position) const {
			return (static_cast<size_t>(chunk_position.x) << 32) | static_cast<size_t>(chunk_position.y);
		}
	};
}
