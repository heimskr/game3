#pragma once

#include "graphics/HasBackbuffer.h"
#include "graphics/RenderOptions.h"
#include "graphics/Shader.h"
#include "types/Types.h"

namespace Game3 {
	class Window;
	class Texture;

	class SpriteRenderer: public HasBackbuffer {
		protected:
			SpriteRenderer(Window &);

		public:
			Window *window = nullptr;
			double centerX = 0;
			double centerY = 0;

			virtual void drawOnMap(const std::shared_ptr<Texture> &, double x, double y, double scale, double angle, double alpha) = 0;
			virtual void drawOnMap(const std::shared_ptr<Texture> &, const RenderOptions &) = 0;
			void drawOnMap(const std::shared_ptr<Texture> &, double x, double y);
			void drawOnMap(const std::shared_ptr<Texture> &);
			virtual void drawOnScreen(GL::Texture &, const RenderOptions &);
			virtual void drawOnScreen(const std::shared_ptr<Texture> &, const RenderOptions &);

			virtual void renderNow() {}

			virtual void reset() = 0;

			template <typename T>
			void operator()(T &texture, const RenderOptions &options) {
				drawOnMap(texture, options);
			}
	};
}