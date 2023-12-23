#pragma once

#include "lib/Eigen.h"

#include "graphics/HasBackbuffer.h"
#include "graphics/Shader.h"
#include "threading/Lockable.h"

namespace Game3 {
	class Canvas;
	struct RenderOptions;

	class CircleRenderer: public HasBackbuffer {
		public:
			Shader shader;

			CircleRenderer(Canvas &);

			~CircleRenderer();

			void reset();
			void update(int width, int height) override;

			void drawOnMap(const RenderOptions &, float cutoff = -1.f);
			void drawOnScreen(const Color &color, float x, float y, float width, float height, float cutoff = -1.f, float angle = 0.f);

			inline auto getWidth()  const { return backbufferWidth;  }
			inline auto getHeight() const { return backbufferHeight; }

		private:
			Canvas &canvas;
			glm::mat4 projection;
			GLuint quadVAO = 0;
			int initializedTo = 0;

			Lockable<std::map<int, std::vector<float>>> vertexMap;

			void initRenderData(int sides);
			const std::vector<float> & getVertices(int sides, std::shared_lock<DefaultMutex> &);
			bool isInitialized() const;
	};
}
