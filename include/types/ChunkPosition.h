#pragma once

#include <cstdint>
#include <functional>
#include <random>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "Constants.h"
#include "types/Position.h"
#include "types/Types.h"
#include "util/Concepts.h"

namespace Game3 {
	class Buffer;

	struct ChunkPosition {
		using IntType = int32_t;

		IntType x = 0;
		IntType y = 0;

		std::default_random_engine getRNG() const;

		ChunkPosition() = default;
		ChunkPosition(IntType x_, IntType y_);
		explicit ChunkPosition(const Position &);
		explicit ChunkPosition(std::string_view);

		inline bool operator==(const ChunkPosition &other) const {
			return this == &other || (x == other.x && y == other.y);
		}

		auto operator<=>(const ChunkPosition &) const = default;

		ChunkPosition operator+(const ChunkPosition &) const;
		ChunkPosition operator-(const ChunkPosition &) const;
		ChunkPosition & operator+=(const ChunkPosition &);
		ChunkPosition & operator-=(const ChunkPosition &);

		explicit operator std::string() const;

		template <typename Fn>
		void iterate(const Fn &fn) const {
			for (int64_t row = y * CHUNK_SIZE, row_max = (y + 1) * CHUNK_SIZE; row < row_max; ++row)
				for (int64_t column = x * CHUNK_SIZE, column_max = (x + 1) * CHUNK_SIZE; column < column_max; ++column)
					fn(Position{row, column});
		}

		Position topLeft() const {
			return {y * CHUNK_SIZE, x * CHUNK_SIZE};
		}

		Position bottomRight() const {
			return {(y + 1) * CHUNK_SIZE - 1, (x + 1) * CHUNK_SIZE - 1};
		}
	};

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
		public:
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
			explicit operator std::string() const;

			template <typename Fn>
			requires(Returns<Fn, void, ChunkPosition>)
			void iterate(const Fn &fn) const {
				for (auto y = topLeft.y; y <= bottomRight.y; ++y)
					for (auto x = topLeft.x; x <= bottomRight.x; ++x)
						fn(ChunkPosition{x, y});
			}

			/** Stops iterating if the function returns true. */
			template <typename Fn>
			requires(Returns<Fn, bool, ChunkPosition>)
			void iterate(const Fn &fn) const {
				for (auto y = topLeft.y; y <= bottomRight.y; ++y)
					for (auto x = topLeft.x; x <= bottomRight.x; ++x)
						if (fn(ChunkPosition{x, y}))
							return;
			}
	};

	void from_json(const nlohmann::json &, ChunkRange &);
	void to_json(nlohmann::json &, const ChunkRange &);
}

template <>
struct std::hash<Game3::ChunkPosition> {
	size_t operator()(const Game3::ChunkPosition &chunk_position) const {
		return std::hash<uint64_t>{}((static_cast<uint64_t>(chunk_position.x) << 32) | static_cast<uint64_t>(chunk_position.y));
	}
};

template <>
struct std::formatter<Game3::ChunkPosition> {
	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
    }

	auto format(const auto &chunk_position, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "({}, {})", chunk_position.x, chunk_position.y);
	}
};

template <>
struct std::formatter<Game3::ChunkRange> {
	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
    }

	auto format(const auto &range, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "[{}, {}]", range.topLeft, range.bottomRight);
	}
};
