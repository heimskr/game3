#pragma once

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/glcanvas.h>

#include "Texture.h"
#include "ui/TilemapRenderer.h"

namespace Game3 {
	class Canvas: public nanogui::GLCanvas {
		public:
			Canvas(nanogui::Widget *parent);

			~Canvas();

			void drawImage(const Texture &, const nanogui::Vector2f &screen_pos,
			               const nanogui::Vector2f &image_offset = {0.f, 0.f},
			               const nanogui::Vector2f &image_extent = {-1.f, -1.f});

			void drawGL() override;

			nanogui::Vector2f & center() { return tilemapRenderer.center; }

		private:
			Texture grass;
			// nanogui::GLShader shader;
			std::shared_ptr<Tilemap> tilemap;
			TilemapRenderer tilemapRenderer;
	};
}
