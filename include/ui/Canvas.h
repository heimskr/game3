#pragma once

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/glcanvas.h>

// #include <ft2build.h>
// #include FT_FREETYPE_H

#include "Texture.h"
#include "ui/GeometryRenderer.h"
#include "ui/ElementBufferedRenderer.h"

namespace Game3 {
	class Canvas: public nanogui::GLCanvas {
		public:
			Canvas(nanogui::Widget *parent);

			~Canvas();

			// void drawImage(const Texture &, const nanogui::Vector2f &screen_pos, const nanogui::Vector2f &image_offset = {0.f, 0.f}, const nanogui::Vector2f &image_extent = {-1.f, -1.f});

			void draw(NVGcontext *) override;
			void drawGL() override;
			bool scrollEvent(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;

			nanogui::Vector2f & center() { return tilemapRenderer.center; }

		private:
			NVGcontext *context = nullptr;
			Texture grass;
			// nanogui::GLShader shader;
			std::shared_ptr<Tilemap> tilemap;
			GeometryRenderer tilemapRenderer;
			// ElementBufferedRenderer tilemapRenderer;
			int font = -1;

			// FT_Library ftLibrary;
			// FT_Face face;
			// nanogui::GLShader textShader;
	};
}
