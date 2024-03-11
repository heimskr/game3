#pragma once

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite_renderer.h

#include "graphics/HasBackbuffer.h"
#include "graphics/OpenGL.h"

namespace Game3 {
	class Canvas;
	class Texture;

	class Recolor: public HasBackbuffer {
		public:
			Canvas *canvas = nullptr;
			Shader shader;
			double centerX = 0;
			double centerY = 0;

			Recolor(Canvas &);
			Recolor(const Recolor &) = delete;
			Recolor(Recolor &&);

			~Recolor();

			Recolor & operator=(const Recolor &) = delete;
			Recolor & operator=(Recolor &&);

			void remove();
			void update(const Canvas &);
			void update(int width, int height);

			void drawOnMap(const std::shared_ptr<Texture> &, const std::shared_ptr<Texture> &mask, const RenderOptions &, float hue, float saturation, float value_multiplier);

			void reset();

		private:
			GLuint quadVAO = 0;
			bool initialized = false;

			void initRenderData();
			void setupShader(int texture_width, int texture_height, const RenderOptions &);
	};
}