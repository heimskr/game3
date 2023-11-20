#pragma once

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite_renderer.h

#include "graphics/SpriteRenderer.h"

#include <map>
#include <memory>

namespace Game3 {
	class BatchSpriteRenderer: public SpriteRenderer {
		public:
			Shader shader;

			BatchSpriteRenderer(Canvas &);

			BatchSpriteRenderer(const BatchSpriteRenderer &) = delete;
			BatchSpriteRenderer(BatchSpriteRenderer &&) = delete;

			~BatchSpriteRenderer() override;

			BatchSpriteRenderer & operator=(const BatchSpriteRenderer &) = delete;
			BatchSpriteRenderer & operator=(BatchSpriteRenderer &&) = delete;

			void remove();
			void update(const Canvas &) override;

			void drawOnMap(const std::shared_ptr<Texture> &, double x, double y, double scale = 1.f, double angle = 0.f, double alpha = 1.f) override;
			void drawOnMap(const std::shared_ptr<Texture> &, RenderOptions = {}) override;

			void renderNow() override;

			void reset() override;

		private:
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
				size_t lastDataCount = 0;
			};

			std::vector<BatchItem> batchItems;
			std::map<GLuint, Atlas> atlases;

			void initRenderData();
			void flush(std::shared_ptr<Texture> texture, const std::vector<const RenderOptions *> &, size_t tile_size);

			Atlas generateAtlas(std::shared_ptr<Texture>, const std::vector<const RenderOptions *> &);
			std::vector<float> generateData(std::shared_ptr<Texture>, const std::vector<const RenderOptions *> &);
	};
}
