#pragma once

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/TileMap.cs

#include <memory>
#include <vector>

#include <nlohmann/json.hpp>

#include "Position.h"
#include "Texture.h"
#include "Types.h"

namespace Game3 {
	class Tileset;

	struct Tilemap {
		int width = 0;
		int height = 0;
		int tileSize = 0;
		std::shared_ptr<Texture> texture;
		int setWidth = 0;
		int setHeight = 0;
		std::vector<TileID> tiles;
		std::shared_ptr<Tileset> tileset;

		Tilemap() = default;

		Tilemap(int width_, int height_, int tile_size, int set_width, int set_height, std::shared_ptr<Tileset>);

		Tilemap(int width_, int height_, int tile_size, std::shared_ptr<Tileset>);

		void init();

		inline decltype(tiles)::value_type & operator()(int x, int y) {
			return tiles[x + y * width];
		}

		inline const decltype(tiles)::value_type & operator()(int x, int y) const {
			return tiles[x + y * width];
		}

		inline decltype(tiles)::value_type & operator[](const Position &position) {
			return tiles[position.column + position.row * width];
		}

		inline const decltype(tiles)::value_type & operator[](const Position &position) const {
			return tiles[position.column + position.row * width];
		}

		inline decltype(tiles)::value_type & operator[](Index index) {
			return tiles[index];
		}

		inline const decltype(tiles)::value_type & operator[](Index index) const {
			return tiles[index];
		}

		inline size_t size() const { return tiles.size(); }

		std::vector<Index> getLand(Index right_pad = 0, Index bottom_pad = 0) const;
	};

	void to_json(nlohmann::json &, const Tilemap &);
	void from_json(const nlohmann::json &, Tilemap &);

	using TilemapPtr = std::shared_ptr<Tilemap>;
}
