#pragma once

#include "ui/RectangleRenderer.h"
#include "ui/TilemapRenderer.h"

namespace Game3 {
	class ElementBufferedRenderer: public TilemapRenderer {
		public:
			ElementBufferedRenderer() = default;
			virtual ~ElementBufferedRenderer() override;

			void reset();
			void init(const TilemapPtr &, const TileSet &) override;
			void render(float divisor) override;
			void reupload();
			bool onBackbufferResized(int width, int height) override;

			operator bool() const { return initialized; }

		private:
			bool initialized = false;
			GLuint shaderHandle = 0;
			GLuint vboHandle = 0;
			GLuint eboHandle = 0;
			GLuint vaoHandle = 0;
			GLuint lfbHandle = 0;
			GLuint lfbTexture = 0;
			GLuint sampler = 0;
			std::vector<GLint> brightTiles;
			RectangleRenderer rectangle;

			void createShader();
			void generateVertexBufferObject();
			void generateElementBufferObject();
			void generateVertexArrayObject();
			void generateLightingFrameBuffer();
			void generateLightingTexture();
			void generateSampler();
	};
}
