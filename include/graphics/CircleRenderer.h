#pragma once

#include "lib/Eigen.h"

#include "graphics/Shader.h"
#include "threading/Lockable.h"

namespace Game3 {
	class Canvas;
	struct RenderOptions;

	class CircleRenderer {
		public:
			Shader shader;

			CircleRenderer(Canvas &);

			~CircleRenderer();

			void reset();
			void update(int backbuffer_width, int backbuffer_height);

			void drawOnMap(const RenderOptions &, float cutoff = -1.f);
			void drawOnScreen(const Eigen::Vector4f &color, float x, float y, float width, float height, float cutoff = -1.f, float angle = 0.f);

		private:
			Canvas &canvas;
			glm::mat4 projection;
			GLuint quadVAO = 0;
			int initializedTo = 0;
			int backbufferWidth = -1;
			int backbufferHeight = -1;

			Lockable<std::map<int, std::vector<float>>> vertexMap;

			void initRenderData(int sides);
			const std::vector<float> & getVertices(int sides, std::shared_lock<DefaultMutex> &);
			bool isInitialized() const;
	};
}
