#pragma once

#include "graphics/Shader.h"
#include "types/Types.h"

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
		bool invertY = true;
	};

	class SpriteRenderer {
		protected:
			SpriteRenderer(Canvas &);

		public:
			Canvas *canvas;
			double centerX = 0.f;
			double centerY = 0.f;
			int backbufferWidth = -1;
			int backbufferHeight = -1;

			virtual ~SpriteRenderer() = default;

			virtual void update(const Canvas &) = 0;
			virtual void drawOnMap(const std::shared_ptr<Texture> &, double x, double y, double scale, double angle, double alpha) = 0;
			virtual void drawOnMap(const std::shared_ptr<Texture> &, RenderOptions) = 0;
			void drawOnMap(const std::shared_ptr<Texture> &, double x, double y);
			void drawOnMap(const std::shared_ptr<Texture> &);

			virtual void renderNow() {}

			virtual void reset() = 0;

			template <typename T>
			void operator()(T &texture, RenderOptions options) {
				drawOnMap(texture, std::move(options));
			}
	};
}