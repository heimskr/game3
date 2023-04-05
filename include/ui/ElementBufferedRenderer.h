#pragma once

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

			operator bool() const { return initialized; }

		private:
			bool initialized = false;
			GLuint shaderHandle;
			GLuint vboHandle;
			GLuint eboHandle;
			GLuint vaoHandle;
			std::vector<GLint> brightTiles;

			void createShader();
			void generateVertexBufferObject();
			void generateElementBufferObject();
			void generateVertexArrayObject();
	};
}
