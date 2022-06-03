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
#include "ui/GeometryRenderer.h"
#include "ui/ElementBufferedRenderer.h"

namespace Game3 {
	class Canvas: public nanogui::GLCanvas {
		public:
			using Tile = uint8_t;
			constexpr static size_t WIDTH = 256;
			constexpr static size_t HEIGHT = WIDTH;
			constexpr static Tile DEEPER_WATER = 6;
			constexpr static Tile DEEP_WATER = 3;
			constexpr static Tile WATER = 2;
			constexpr static Tile SHALLOW_WATER = 1;
			constexpr static Tile SAND = 4;
			constexpr static Tile LIGHT_GRASS = 11;
			constexpr static Tile GRASS = 12;
			constexpr static Tile GRASS_ALT1 = 40;
			constexpr static Tile GRASS_ALT2 = 41;
			constexpr static Tile GRAY = 5;
			constexpr static Tile ROAD = 15;
			constexpr static Tile DIRT = 16;
			constexpr static Tile TOWER_NW = 50;
			constexpr static Tile TOWER_NE = 51;
			constexpr static Tile TOWER_SW = 52;
			constexpr static Tile TOWER_SE = 53;
			constexpr static Tile TOWER_WE = 54;
			constexpr static Tile TOWER_NS = 55;
			constexpr static Tile TOWER_N = 56;
			constexpr static Tile TOWER_S = 57;

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

			std::vector<unsigned> getLand(uint8_t tiles[HEIGHT][WIDTH], size_t right_pad = 0, size_t bottom_pad = 0) const;
			void createTown(uint8_t layer1[HEIGHT][WIDTH], uint8_t layer2[HEIGHT][WIDTH], size_t index, size_t width, size_t height, size_t pad) const;

			static inline bool isLand(Tile tile) {
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
