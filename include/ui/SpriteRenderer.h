#pragma once

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite_renderer.h

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>

namespace Game3 {
	class Texture;

	class SpriteRenderer {
		public:
			nanogui::GLShader shader;

			SpriteRenderer();
			~SpriteRenderer();

			void backBufferChanged(int width, int height);
			void draw(Texture &, float x, float y, float scale = 1.f, float angle = 0.f, float alpha = 1.f);
			void draw(Texture &, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale = 1.f, float angle = 0.f, float alpha = 1.f);

		private:
			void initRenderData();
			GLuint quadVAO = 0;
			bool initialized = false;
			int backBufferWidth = -1;
			int backBufferHeight = -1;
	};
}
