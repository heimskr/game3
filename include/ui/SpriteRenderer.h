#pragma once

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite_renderer.h

#include "Shader.h"

namespace Game3 {
	class Canvas;
	class Texture;

	class SpriteRenderer {
		public:
			Canvas &canvas;
			Shader shader;

			SpriteRenderer(Canvas &);
			~SpriteRenderer();

			void update(int backbuffer_width, int backbuffer_height);

			void drawOnMap(Texture &, float x, float y, float scale = 1.f, float angle = 0.f, float alpha = 1.f);
			void drawOnMap(Texture &, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale = 1.f, float angle = 0.f, float alpha = 1.f);

			void drawOnScreen(Texture &, float x, float y, float scale = 1.f, float angle = 0.f, float alpha = 1.f);
			void drawOnScreen(Texture &, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale = 1.f, float angle = 0.f, float alpha = 1.f);

		private:
			void initRenderData();
			GLuint quadVAO = 0;
			bool initialized = false;
			int backbufferWidth = -1;
			int backbufferHeight = -1;
	};
}
