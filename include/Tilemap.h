#pragma once

#include <vector>

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/TileMap.cs

namespace Game3 {
	struct Tilemap {
		int width;
		int height;
		int tileSize;
		std::vector<char> tiles;
		int handle = -1;

		Tilemap(int width_, int height_, int tile_size): width(width_), height(height_), tileSize(tile_size) {
			tiles.resize(width * height);
		}

		decltype(tiles)::value_type & operator()(int x, int y) {
			return tiles[x + y * width];
		}
	};
}
