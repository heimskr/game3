#pragma once

#include <Eigen/Eigen>

#include "Shader.h"

namespace Game3 {
	class Canvas;

	class RectangleRenderer {
		public:
			Canvas &canvas;
			Shader shader;

			RectangleRenderer(Canvas &);
			~RectangleRenderer();

			void update(int backbuffer_width, int backbuffer_height);

			void drawOnScreen(const Eigen::Vector4f &color, float x, float y, float width, float height, float angle = 0.f);

			void operator()(const Eigen::Vector4f &color, float x, float y, float width, float height, float angle = 0.f);

		private:
			void initRenderData();
			GLuint quadVAO = 0;
			bool initialized = false;
			int backbufferWidth = -1;
			int backbufferHeight = -1;
	};
}
