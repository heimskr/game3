#pragma once

#include "ui/TilemapRenderer.h"

namespace Game3 {
	class ElementBufferedRenderer: public TilemapRenderer {
		public:
			ElementBufferedRenderer() = default;
			virtual ~ElementBufferedRenderer() override;

			void reset();
			void initialize(const std::shared_ptr<Tilemap> &) override;
			void render() override;
			void reupload();

			operator bool() const { return initialized; }

		private:
			bool initialized = false;
			GLuint shaderHandle;
			GLuint vboHandle;
			GLuint eboHandle;
			GLuint vaoHandle;

			void createShader();
			void generateVertexBufferObject();
			void generateElementBufferObject();
			void generateVertexArrayObject();
	};
}
