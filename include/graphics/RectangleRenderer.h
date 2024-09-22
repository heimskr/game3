#pragma once

#include "lib/Eigen.h"

#include "graphics/HasBackbuffer.h"
#include "graphics/Shader.h"

namespace Game3 {
	class Window;
	struct Color;
	struct Rectangle;
	struct RenderOptions;

	class RectangleRenderer: public HasBackbuffer {
		public:
			Shader shader;

			RectangleRenderer(Window &);

			~RectangleRenderer();

			void reset();
			void update(int width, int height) override;

			void drawOnMap(const RenderOptions &);
			void drawOnScreen(const Color &, float x, float y, float width, float height, float angle = 0.f);
			void drawOnScreen(const Color &, const Rectangle &, float angle = 0.f);

			void operator()(const Color &, float x, float y, float width, float height, float angle = 0.f);
			void operator()(const Color &, const Rectangle &, float angle = 0.f);

		private:
			Window &window;
			glm::mat4 projection;
			GLuint quadVAO = 0;
			bool initialized = false;

			void initRenderData();
	};
}
