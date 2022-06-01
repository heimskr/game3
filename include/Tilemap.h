#pragma once

#include <vector>

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/TileMap.cs

namespace Game3 {
	struct Tilemap {
		int width;
		int height;
		int tileSize;
		std::vector<uint8_t> tiles;
		std::vector<uint8_t> sums;
		int handle;

		Tilemap(int width_, int height_, int tile_size, int handle_):
		width(width_), height(height_), tileSize(tile_size), handle(handle_) {
			tiles.resize(width * height);
			sums.resize(width * height);
		}

		decltype(tiles)::value_type & operator()(int x, int y) {
			return tiles[x + y * width];
		}
	};
}
