#pragma once

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/TileMap.cs

#include <memory>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "types/Position.h"
#include "graphics/Texture.h"
#include "types/Types.h"

namespace Game3 {
	struct BiomeMap {
		int width = 0;
		int height = 0;
		std::vector<BiomeType> tiles;

		BiomeMap() = default;

		BiomeMap(int width_, int height_): width(width_), height(height_) {
			tiles.resize(width * height);
		}

		BiomeMap(int width_, int height_, BiomeType fill): width(width_), height(height_) {
			tiles.resize(width * height, fill);
		}

		inline void fill(BiomeType value) {
			tiles.assign(tiles.size(), value);
		}

		inline decltype(tiles)::value_type & operator()(int x, int y) {
			return tiles[x + y * width];
		}

		inline const decltype(tiles)::value_type & operator()(int x, int y) const {
			return tiles[x + y * width];
		}

		inline decltype(tiles)::value_type & operator()(const Position &position) {
			return tiles[position.column + position.row * width];
		}

		inline const decltype(tiles)::value_type & operator()(const Position &position) const {
			return tiles[position.column + position.row * width];
		}
	};

	void to_json(nlohmann::json &, const BiomeMap &);
	void from_json(const nlohmann::json &, BiomeMap &);

	using BiomeMapPtr = std::shared_ptr<BiomeMap>;
}
