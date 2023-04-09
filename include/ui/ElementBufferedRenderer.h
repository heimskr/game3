#pragma once

#include "Shader.h"
#include "ui/RectangleRenderer.h"
#include "ui/Reshader.h"
#include "ui/TilemapRenderer.h"
#include "util/GL.h"

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
			Shader shader {"terrain"};
			GL::FloatVAO vao;
			GLuint vboHandle = 0;
			GL::EBO ebo;
			GLuint lfbHandle = 0;
			GLuint lfbTexture = 0;
			GLuint lfbBlurredTexture = 0;
			std::vector<GLint> brightTiles;
			std::unordered_set<TileID> brightSet;
			RectangleRenderer rectangle;
			Reshader reshader;
			Realm *realm = nullptr;
			std::vector<TileID> tileCache;

			void generateVertexBufferObject();
			void generateElementBufferObject();
			void generateVertexArrayObject();
			void generateLightingFrameBuffer();
			void generateLightingTexture();
			void generateSampler();

			void recomputeLighting();
	};
}
