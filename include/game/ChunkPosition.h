#pragma once

#include <cstdint>
#include <functional>
#include <ostream>
#include <random>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "Constants.h"
#include "Position.h"
#include "Types.h"

namespace Game3 {
	class Buffer;

	struct ChunkPosition {
		int32_t x = 0;
		int32_t y = 0;

		std::default_random_engine getRNG() const;

		ChunkPosition() = default;
		ChunkPosition(int32_t x_, int32_t y_);
		explicit ChunkPosition(const Position &);

		inline bool operator==(const ChunkPosition &other) const {
			return this == &other || (x == other.x && y == other.y);
		}

		auto operator<=>(const ChunkPosition &) const = default;

		explicit operator std::string() const;

		template <typename Fn>
		void iterate(const Fn &fn) const {
			for (auto row = y * CHUNK_SIZE, row_max = (y + 1) * CHUNK_SIZE; row < row_max; ++row)
				for (auto column = x * CHUNK_SIZE, column_max = (x + 1) * CHUNK_SIZE; column < column_max; ++column)
					fn(Position{row, column});
		}
	};

	std::ostream & operator<<(std::ostream &, ChunkPosition);
	Buffer & operator+=(Buffer &, const ChunkPosition &);
	Buffer & operator<<(Buffer &, const ChunkPosition &);
	Buffer & operator>>(Buffer &, ChunkPosition &);

	struct ChunkRequest {
		ChunkPosition position;
		uint64_t counterThreshold;

		ChunkRequest(ChunkPosition position_, uint64_t counter_threshold = 0):
			position(position_), counterThreshold(counter_threshold) {}

		auto operator<=>(const ChunkRequest &) const = default;
	};

	void from_json(const nlohmann::json &, ChunkPosition &);
	void to_json(nlohmann::json &, const ChunkPosition &);

	struct ChunkRange {
		ChunkPosition topLeft;
		ChunkPosition bottomRight;

		ChunkRange(ChunkPosition top_left, ChunkPosition bottom_right);
		ChunkRange(ChunkPosition);

		inline Index tileWidth() const  { return (bottomRight.x - topLeft.x + 1) * CHUNK_SIZE; }
		inline Index tileHeight() const { return (bottomRight.y - topLeft.y + 1) * CHUNK_SIZE; }

		inline Index rowMin() const { return topLeft.y * CHUNK_SIZE; }
		/** Compare with <=, not <. */
		inline Index rowMax() const { return (bottomRight.y + 1) * CHUNK_SIZE - 1; }
		inline Index columnMin() const { return topLeft.x * CHUNK_SIZE; }
		/** Compare with <=, not <. */
		inline Index columnMax() const { return (bottomRight.x + 1) * CHUNK_SIZE - 1; }

		inline bool contains(ChunkPosition chunk_position) const {
			return topLeft.x <= chunk_position.x && chunk_position.x <= bottomRight.x && topLeft.y <= chunk_position.y && chunk_position.y <= bottomRight.y;
		}

		auto operator<=>(const ChunkRange &) const = default;

		template <typename Fn>
		void iterate(const Fn &fn) const {
			for (auto y = topLeft.y; y <= bottomRight.y; ++y)
				for (auto x = topLeft.x; x <= bottomRight.x; ++x)
					fn(ChunkPosition{x, y});
		}
	};

	void from_json(const nlohmann::json &, ChunkRange &);
	void to_json(nlohmann::json &, const ChunkRange &);
}

namespace std {
	template <>
	struct hash<Game3::ChunkPosition> {
		size_t operator()(const Game3::ChunkPosition &chunk_position) const {
			return (static_cast<size_t>(chunk_position.x) << 32) | static_cast<size_t>(chunk_position.y);
		}
	};
}
