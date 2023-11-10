#pragma once

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite_renderer.h

#include "graphics/Shader.h"

#include <map>

namespace GL {
	class Texture;
}

namespace Game3 {
	class Canvas;
	class Texture;

	struct RenderOptions {
		double x = 0.f;
		double y = 0.f;
		double offsetX = 0.f;
		double offsetY = 0.f;
		double sizeX = 16.f;
		double sizeY = 16.f;
		double scaleX = 1.f;
		double scaleY = 1.f;
		double angle = 0.f;
		Color color{1.f, 1.f, 1.f, 1.f};
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
			SpriteRenderer(SpriteRenderer &&) = delete;

			~SpriteRenderer();

			SpriteRenderer & operator=(const SpriteRenderer &) = delete;
			SpriteRenderer & operator=(SpriteRenderer &&) = delete;

			void remove();
			void update(const Canvas &);

			void drawOnMap(const std::shared_ptr<Texture> &, double x, double y, double scale = 1.f, double angle = 0.f, double alpha = 1.f);
			void drawOnMap(const std::shared_ptr<Texture> &, RenderOptions = {});

			template <typename T>
			void operator()(T &texture, RenderOptions options) {
				drawOnMap(texture, std::move(options));
			}

			void renderNow();

			void reset();

		private:
			GLuint quadVAO = 0;
			bool initialized = false;
			double canvasScale = -1.;

			struct BatchItem {
				std::shared_ptr<Texture> texture;
				RenderOptions options;
			};

			struct Atlas {
				std::shared_ptr<Texture> texture;
				GL::VBO vbo;
				GL::FloatVAO vao;
			};

			std::vector<BatchItem> batchItems;
			std::map<GLuint, Atlas> atlases;

			void initRenderData();
			void flush(std::shared_ptr<Texture> texture, const std::vector<const RenderOptions *> &);
			double hackY(double y, double y_offset, double scale);
			void draw(const Atlas &atlas);

			Atlas generateAtlas(std::shared_ptr<Texture>, const std::vector<const RenderOptions *> &);
			std::vector<float> generateData(std::shared_ptr<Texture>, const std::vector<const RenderOptions *> &);
	};
}
