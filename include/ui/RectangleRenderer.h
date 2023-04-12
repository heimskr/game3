#pragma once

#include <Eigen/Eigen>

#include "Shader.h"

namespace Game3 {

	class RectangleRenderer {
		public:
			Shader shader;

			RectangleRenderer();
			~RectangleRenderer();

			void reset();
			void update(int backbuffer_width, int backbuffer_height);

			void drawOnScreen(const Eigen::Vector4f &color, float x, float y, float width, float height, float angle = 0.f);

			void operator()(const Eigen::Vector4f &color, float x, float y, float width, float height, float angle = 0.f);

		private:
			void initRenderData();
			glm::mat4 projection;
			GLuint quadVAO = 0;
			bool initialized = false;
			int backbufferWidth = -1;
			int backbufferHeight = -1;
	};
}
