#pragma once

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/TileMap.cs

#include <vector>

#include <nlohmann/json.hpp>

#include "Texture.h"

namespace Game3 {
	struct Tilemap {
		int width;
		int height;
		int tileSize;
		int setWidth;
		int setHeight;
		std::vector<uint8_t> tiles;
		Texture texture;

		Tilemap(int width_, int height_, int tile_size, int set_width, int set_height, const std::filesystem::path &path):
		width(width_), height(height_), tileSize(tile_size), setWidth(set_width), setHeight(set_height), texture(path) {
			tiles.resize(width * height);
		}

		Tilemap(int width_, int height_, int tile_size, const Texture &texture_):
		width(width_), height(height_), tileSize(tile_size), setWidth(texture_.width), setHeight(texture_.height), texture(texture_) {
			tiles.resize(width * height);
		}

		decltype(tiles)::value_type & operator()(int x, int y) {
			return tiles[x + y * width];
		}
	};

	void to_json(nlohmann::json &, const Tilemap &);
}
