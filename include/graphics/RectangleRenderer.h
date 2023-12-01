#pragma once

#include "lib/Eigen.h"

#include "graphics/Shader.h"

namespace Game3 {
	class Canvas;
	struct RenderOptions;

	class RectangleRenderer {
		public:
			Shader shader;

			RectangleRenderer(Canvas &);

			~RectangleRenderer();

			void reset();
			void update(int backbuffer_width, int backbuffer_height);

			void drawOnMap(const RenderOptions &);
			void drawOnScreen(const Eigen::Vector4f &color, float x, float y, float width, float height, float angle = 0.f);

			void operator()(const Eigen::Vector4f &color, float x, float y, float width, float height, float angle = 0.f);

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
