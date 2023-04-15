#pragma once

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite_renderer.h

#include "Shader.h"

namespace GL {
	class Texture;
}

namespace Game3 {
	class Canvas;
	class Texture;

	struct RenderOptions {
		float x = 0.f;
		float y = 0.f;
		float x_offset = 0.f;
		float y_offset = 0.f;
		float size_x = 16.f;
		float size_y = 16.f;
		float scale = 1.f;
		float angle = 0.f;
		float alpha = 1.f;
		bool hackY = true;
		bool invertY = true;
	};

	class SpriteRenderer {
		public:
			Canvas *canvas = nullptr;
			Shader shader;

			SpriteRenderer(Canvas &);
			~SpriteRenderer();

			SpriteRenderer & operator=(SpriteRenderer &&);

			void remove();
			void update(int backbuffer_width, int backbuffer_height);

			void drawOnMap(Texture &, float x, float y, float scale = 1.f, float angle = 0.f, float alpha = 1.f);
			void drawOnMap(Texture &, RenderOptions = {});
			void drawOnMap(GL::Texture &, RenderOptions = {});

			void drawOnScreen(Texture &, float x, float y, float scale = 1.f, float angle = 0.f, float alpha = 1.f);
			void drawOnScreen(Texture &, RenderOptions = {});
			void drawOnScreen(GL::Texture &, RenderOptions = {});

			// void drawOnMap(Texture &, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale = 1.f, float angle = 0.f, float alpha = 1.f);
			// void drawOnMap(GL::Texture &, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale = 1.f, float angle = 0.f, float alpha = 1.f);
			// void drawOnScreen(Texture &, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale = 1.f, float angle = 0.f, float alpha = 1.f);
			// void drawOnScreen(GL::Texture &, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale = 1.f, float angle = 0.f, float alpha = 1.f);

			template <typename T>
			void operator()(T &texture, RenderOptions &&options) {
				drawOnScreen(texture, std::forward<RenderOptions>(options));
			}

			void reset();

		private:
			GLuint quadVAO = 0;
			bool initialized = false;
			int backbufferWidth = -1;
			int backbufferHeight = -1;

			void initRenderData();
			void setupShader(int texture_width, int texture_height, const RenderOptions &);
			void hackY(float &y, float y_offset, float scale);
	};
}
