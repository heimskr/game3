#pragma once

#include "lib/Eigen.h"

#include "graphics/HasBackbuffer.h"
#include "graphics/Shader.h"

namespace Game3 {
	class Canvas;
	struct Color;
	struct RenderOptions;

	class RectangleRenderer: public HasBackbuffer {
		public:
			Shader shader;

			RectangleRenderer(Canvas &);

			~RectangleRenderer();

			void reset();
			void update(int width, int height) override;

			void drawOnMap(const RenderOptions &);
			void drawOnScreen(const Color &color, float x, float y, float width, float height, float angle = 0.f);

			void operator()(const Color &color, float x, float y, float width, float height, float angle = 0.f);

		private:
			Canvas &canvas;
			glm::mat4 projection;
			GLuint quadVAO = 0;
			bool initialized = false;
			int backbufferWidth = -1;
			int backbufferHeight = -1;

			void initRenderData();
	};
}
