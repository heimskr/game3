#pragma once

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite_renderer.h

#include "graphics/SpriteRenderer.h"

namespace GL {
	class Texture;
}

namespace Game3 {
	class Canvas;
	class Texture;

	class SingleSpriteRenderer: public SpriteRenderer {
		public:
			Shader shader;

			SingleSpriteRenderer(Canvas &);
			SingleSpriteRenderer(const SingleSpriteRenderer &) = delete;
			SingleSpriteRenderer(SingleSpriteRenderer &&);

			~SingleSpriteRenderer();

			SingleSpriteRenderer & operator=(const SingleSpriteRenderer &) = delete;
			SingleSpriteRenderer & operator=(SingleSpriteRenderer &&);

			void remove();
			void update(const Canvas &) override;
			void update(int width, int height) override;

			void drawOnMap(const std::shared_ptr<Texture> &, double x, double y, double scale, double angle, double alpha) override;
			void drawOnMap(const std::shared_ptr<Texture> &, const RenderOptions &) override;
			void drawOnMap(GL::Texture &, const RenderOptions &);

			void drawOnScreen(GL::Texture &, const RenderOptions &) override;
			void drawOnScreen(const std::shared_ptr<Texture> &, const RenderOptions &) override;

			void reset() override;

		private:
			GLuint quadVAO = 0;
			bool initialized = false;
			unsigned vbo = -1;
			float lastXCoord = 1;
			float lastYCoord = 1;

			void initRenderData();
			void allowRepeating(const RenderOptions &options, int texture_width, int texture_height);
			void setupShader(int texture_width, int texture_height, const RenderOptions &);
	};
}