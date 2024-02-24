#pragma once

#include "lib/Eigen.h"

#include "graphics/HasBackbuffer.h"
#include "graphics/Shader.h"

namespace Game3 {
	class Canvas;
	struct RenderOptions;

	class RectangleRenderer: public HasBackbuffer {
		public:
			Shader shader;

			RectangleRenderer(Canvas &);

			~RectangleRenderer();

			void reset();
			void update(int width, int height) override;

			void drawOnMap(const RenderOptions &);
			void drawOnScreen(const Eigen::Vector4f &color, double x, double y, double width, double height, double angle = 0.);

			void operator()(const Eigen::Vector4f &color, double x, double y, double width, double height, double angle = 0.);

		private:
			Canvas &canvas;
			glm::dmat4 projection;
			GLuint quadVAO = 0;
			bool initialized = false;
			int backbufferWidth = -1;
			int backbufferHeight = -1;

			void initRenderData();
	};
}
