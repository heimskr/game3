#pragma once

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/TileMap.cs

#include <memory>
#include <vector>

#include <nlohmann/json.hpp>

#include "Position.h"
#include "Texture.h"
#include "Types.h"

namespace Game3 {
	struct Tilemap {
		int width = 0;
		int height = 0;
		int tileSize = 0;
		int setWidth = 0;
		int setHeight = 0;
		std::vector<TileID> tiles;
		Texture texture;

		Tilemap() = default;

		Tilemap(int width_, int height_, int tile_size, int set_width, int set_height, const std::filesystem::path &path):
		width(width_), height(height_), tileSize(tile_size), setWidth(set_width), setHeight(set_height), texture(path) {
			tiles.resize(width * height);
		}

		Tilemap(int width_, int height_, int tile_size, const Texture &texture_):
		width(width_), height(height_), tileSize(tile_size), setWidth(*texture_.width), setHeight(*texture_.height), texture(texture_) {
			tiles.resize(width * height);
		}

		void init() {
			texture.init();
			setWidth = *texture.width;
			setHeight = *texture.height;
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

		inline size_t size() const { return tiles.size(); }

		std::vector<Index> getLand(RealmType type, Index right_pad = 0, Index bottom_pad = 0) const;
	};

	void to_json(nlohmann::json &, const Tilemap &);
	void from_json(const nlohmann::json &, Tilemap &);

	using TilemapPtr = std::shared_ptr<Tilemap>;
}
