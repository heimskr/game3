#pragma once

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
			Canvas(nanogui::Widget *parent);

			// void drawImage(const Texture &, const nanogui::Vector2f &screen_pos, const nanogui::Vector2f &image_offset = {0.f, 0.f}, const nanogui::Vector2f &image_extent = {-1.f, -1.f});

			void draw(NVGcontext *) override;
			void drawGL() override;
			bool scrollEvent(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;
			bool mouseDragEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) override;
			bool mouseButtonEvent(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;

			nanogui::Vector2f & center() { return tilemapRenderer.center; }
			float & scale() { return tilemapRenderer.scale; }

		private:
			constexpr static float HEADER_HEIGHT = 56.f;
			float magic = 8.f;

			NVGcontext *context = nullptr;
			Texture tileset;
			std::shared_ptr<Tilemap> tilemap;
			ElementBufferedRenderer tilemapRenderer;
			int font = -1;
	};
}
