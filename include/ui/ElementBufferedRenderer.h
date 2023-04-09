#pragma once

#include "Shader.h"
#include "ui/RectangleRenderer.h"
#include "ui/Reshader.h"
#include "ui/TilemapRenderer.h"

namespace Game3 {
	class ElementBufferedRenderer: public TilemapRenderer {
		public:
			ElementBufferedRenderer();
			virtual ~ElementBufferedRenderer() override;

			void reset();
			void init(TilemapPtr, const TileSet &) override;
			void render(float divisor) override;
			void reupload();
			bool onBackbufferResized(int width, int height) override;
			inline void markDirty(Realm *realm_) { dirty = true; realm = realm_; }

			operator bool() const { return initialized; }

		private:
			bool initialized = false;
			/** Whether lighting needs to be recomputed. */
			bool dirty = true;
			GLuint shaderHandle = 0;
			GLuint vboHandle = 0;
			GLuint eboHandle = 0;
			GLuint vaoHandle = 0;
			GLuint lfbHandle = 0;
			GLuint lfbTexture = 0;
			GLuint lfbBlurredTexture = 0;
			std::vector<GLint> brightTiles;
			std::unordered_set<TileID> brightSet;
			RectangleRenderer rectangle;
			Reshader reshader;
			Realm *realm = nullptr;
			std::vector<TileID> tileCache;

			void createShader();
			void generateVertexBufferObject();
			void generateElementBufferObject();
			void generateVertexArrayObject();
			void generateLightingFrameBuffer();
			void generateLightingTexture();
			void generateSampler();

			void recomputeLighting();
	};
}
