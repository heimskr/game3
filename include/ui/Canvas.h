#pragma once

#include <random>

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/glcanvas.h>

// #include <ft2build.h>
// #include FT_FREETYPE_H

#include "Image.h"
#include "Texture.h"
#include "Types.h"
#include "ui/GeometryRenderer.h"
#include "ui/ElementBufferedRenderer.h"

namespace Game3 {
	class Canvas: public nanogui::GLCanvas {
		public:
			constexpr static size_t WIDTH = 256;
			constexpr static size_t HEIGHT = WIDTH;
			constexpr static TileID EMPTY = 0;
			constexpr static TileID DEEPER_WATER = 6;
			constexpr static TileID DEEP_WATER = 3;
			constexpr static TileID WATER = 2;
			constexpr static TileID SHALLOW_WATER = 1;
			constexpr static TileID SAND = 4;
			constexpr static TileID LIGHT_GRASS = 11;
			constexpr static TileID GRASS = 12;
			constexpr static TileID GRASS_ALT1 = 40;
			constexpr static TileID GRASS_ALT2 = 41;
			constexpr static TileID GRAY = 5;
			constexpr static TileID ROAD = 15;
			constexpr static TileID DIRT = 16;
			constexpr static TileID TOWER_NW = 50;
			constexpr static TileID TOWER_NE = 51;
			constexpr static TileID TOWER_SW = 52;
			constexpr static TileID TOWER_SE = 53;
			constexpr static TileID TOWER_WE = 54;
			constexpr static TileID TOWER_NS = 55;
			constexpr static TileID TOWER_N = 56;
			constexpr static TileID TOWER_S = 57;
			constexpr static TileID HOUSE1 = 60;
			constexpr static TileID HOUSE2 = 61;
			constexpr static TileID HOUSE3 = 62;

			Canvas(nanogui::Widget *parent);

			// void drawImage(const Texture &, const nanogui::Vector2f &screen_pos, const nanogui::Vector2f &image_offset = {0.f, 0.f}, const nanogui::Vector2f &image_extent = {-1.f, -1.f});

			void draw(NVGcontext *) override;
			void drawGL() override;
			bool scrollEvent(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;
			bool mouseDragEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) override;
			bool mouseButtonEvent(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;

			const nanogui::Vector2f & center() { return tilemapRenderer1.center; }
			float scale() { return tilemapRenderer1.scale; }
			void center(const nanogui::Vector2f &center_) { tilemapRenderer1.center = tilemapRenderer2.center = center_; }
			void scale(float scale_) { tilemapRenderer1.scale = tilemapRenderer2.scale = scale_; }

		private:
			constexpr static float HEADER_HEIGHT = 56.f;
			float magic = 8.f;

			NVGcontext *context = nullptr;
			Texture tileset;
			std::shared_ptr<Tilemap> tilemap1, tilemap2;
			ElementBufferedRenderer tilemapRenderer1, tilemapRenderer2;
			int font = -1;

			std::vector<unsigned> getLand(TileID tiles[HEIGHT][WIDTH], size_t right_pad = 0, size_t bottom_pad = 0) const;
			void createTown(TileID layer1[HEIGHT][WIDTH], TileID layer2[HEIGHT][WIDTH], size_t index, size_t width, size_t height, size_t pad) const;

			static inline bool isLand(TileID tile) {
				switch (tile) {
					case SAND:
					case LIGHT_GRASS:
					case GRASS:
					case GRASS_ALT1:
					case GRASS_ALT2:
						return true;
					default:
						return false;
				}
			}
	};
}
