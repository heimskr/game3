#pragma once

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/TileMap.cs

#include <memory>
#include <vector>

#include <nlohmann/json.hpp>

#include "Position.h"
#include "Texture.h"
#include "Types.h"

namespace Game3 {
	class Quadtree;
	class Tileset;

	class Tilemap {
		public:
			int width = 0;
			int height = 0;
			int tileSize = 0;
			Identifier textureName;
			int setWidth = -1;
			int setHeight = -1;
			std::vector<TileID> tiles;
			std::shared_ptr<Tileset> tileset;
			/** This is a shared pointer for the sake of the copy constructor so that fromJSON can work.
			 *  Please be sure not to let the lifetime of lavaQuadtree exceed the lifetime of this Tilemap. */
			std::shared_ptr<Quadtree> lavaQuadtree;

			Tilemap() = delete;
			Tilemap(int width_, int height_, int tile_size, int set_width, int set_height, std::shared_ptr<Tileset>);
			Tilemap(int width_, int height_, int tile_size, std::shared_ptr<Tileset>);
			~Tilemap();

			void init(const Game &);
			std::vector<Index> getLand(Index right_pad = 0, Index bottom_pad = 0) const;
			std::shared_ptr<Texture> getTexture(const Game &);

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


			static Tilemap fromJSON(const Game &, const nlohmann::json &);

		private:
			std::shared_ptr<Texture> texture;
	};

	void to_json(nlohmann::json &, const Tilemap &);

	using TilemapPtr = std::shared_ptr<Tilemap>;
}
