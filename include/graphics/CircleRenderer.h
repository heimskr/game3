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
			void drawOnScreen(const Color &color, double x, double y, double width, double height, float cutoff = -1.f, double angle = 0.);

			inline auto getWidth()  const { return backbufferWidth;  }
			inline auto getHeight() const { return backbufferHeight; }

		private:
			Canvas &canvas;
			glm::dmat4 projection;
			GLuint quadVAO = 0;
			int initializedTo = 0;

			Lockable<std::map<int, std::vector<double>>> vertexMap;

			void initRenderData(int sides);
			const std::vector<double> & getVertices(int sides, std::shared_lock<DefaultMutex> &);
			bool isInitialized() const;
	};
}
