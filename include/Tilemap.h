#pragma once

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/TileMap.cs

#include <vector>

#include <nlohmann/json.hpp>

#include "Texture.h"
#include "Types.h"

namespace Game3 {
	struct Tilemap {
		int width = 0;
		int height = 0;
		int tileSize = 0;
		int setWidth = 0;
		int setHeight = 0;
		std::vector<uint8_t> tiles;
		Texture texture;

		Tilemap() = default;

		Tilemap(int width_, int height_, int tile_size, int set_width, int set_height, const std::filesystem::path &path):
		width(width_), height(height_), tileSize(tile_size), setWidth(set_width), setHeight(set_height), texture(path) {
			tiles.resize(width * height);
		}

		Tilemap(int width_, int height_, int tile_size, const Texture &texture_):
		width(width_), height(height_), tileSize(tile_size), setWidth(texture_.width), setHeight(texture_.height), texture(texture_) {
			tiles.resize(width * height);
		}

		void init() {
			texture.init();
			setWidth = texture.width;
			setHeight = texture.height;
		}

		decltype(tiles)::value_type & operator()(int x, int y) {
			return tiles[x + y * width];
		}

		std::vector<Index> getLand(RealmType type, size_t right_pad = 0, size_t bottom_pad = 0) const;
	};

	void to_json(nlohmann::json &, const Tilemap &);
	void from_json(const nlohmann::json &, Tilemap &);
}
