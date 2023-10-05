#pragma once

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite_renderer.h

#include "graphics/Shader.h"

namespace GL {
	class Texture;
}

namespace Game3 {
	class Canvas;
	class Texture;

	struct RenderOptions {
		double x = 0.f;
		double y = 0.f;
		double xOffset = 0.f;
		double yOffset = 0.f;
		double sizeX = 16.f;
		double sizeY = 16.f;
		double scaleX = 1.f;
		double scaleY = 1.f;
		double angle = 0.f;
		double alpha = 1.f;
		bool hackY = true;
		bool invertY = true;
	};

	class SpriteRenderer {
		public:
			Canvas *canvas = nullptr;
			Shader shader;
			double divisor = 1.f;
			double centerX = 0.f;
			double centerY = 0.f;
			int backbufferWidth = -1;
			int backbufferHeight = -1;

			SpriteRenderer(Canvas &);
			SpriteRenderer(const SpriteRenderer &) = delete;
			SpriteRenderer(SpriteRenderer &&);

			~SpriteRenderer();

			SpriteRenderer & operator=(const SpriteRenderer &) = delete;
			SpriteRenderer & operator=(SpriteRenderer &&);

			void remove();
			void update(int backbuffer_width, int backbuffer_height);

			void drawOnMap(Texture &, double x, double y, double scale = 1.f, double angle = 0.f, double alpha = 1.f);
			void drawOnMap(Texture &, RenderOptions = {});
			void drawOnMap(GL::Texture &, RenderOptions = {});

			void drawOnScreen(Texture &, double x, double y, double scale = 1.f, double angle = 0.f, double alpha = 1.f);
			void drawOnScreen(Texture &, RenderOptions = {});
			void drawOnScreen(GL::Texture &, RenderOptions = {});

			// void drawOnMap(Texture &, double x, double y, double x_offset, double y_offset, double size_x, double size_y, double scale = 1.f, double angle = 0.f, double alpha = 1.f);
			// void drawOnMap(GL::Texture &, double x, double y, double x_offset, double y_offset, double size_x, double size_y, double scale = 1.f, double angle = 0.f, double alpha = 1.f);
			// void drawOnScreen(Texture &, double x, double y, double x_offset, double y_offset, double size_x, double size_y, double scale = 1.f, double angle = 0.f, double alpha = 1.f);
			// void drawOnScreen(GL::Texture &, double x, double y, double x_offset, double y_offset, double size_x, double size_y, double scale = 1.f, double angle = 0.f, double alpha = 1.f);

			template <typename T>
			void operator()(T &texture, RenderOptions options) {
				drawOnMap(texture, std::move(options));
			}

			void reset();

		private:
			GLuint quadVAO = 0;
			bool initialized = false;

			void initRenderData();
			void setupShader(int texture_width, int texture_height, const RenderOptions &);
			void hackY(double &y, double y_offset, double scale);
	};
}
