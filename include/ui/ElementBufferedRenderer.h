#pragma once

#include <unordered_set>
#include <vector>

#include "Shader.h"
#include "ui/RectangleRenderer.h"
#include "ui/Reshader.h"
#include "ui/TilemapRenderer.h"
#include "util/GL.h"

namespace Game3 {
	class ElementBufferedRenderer: public TilemapRenderer {
		public:
			GL::Texture lightTexture;

			ElementBufferedRenderer(Realm &);
			virtual ~ElementBufferedRenderer() override final;

			void reset();
			void init(TilemapPtr) override final;
			void render(float divisor) override final;
			void reupload();
			bool onBackbufferResized(int width, int height) override final;
			inline void markDirty() { dirty = true; }

			operator bool() const { return initialized; }

		private:
			bool initialized = false;
			/** Whether lighting needs to be recomputed. */
			bool dirty = true;
			Shader shader {"terrain"};
			GL::FloatVAO vao;
			GL::VBO vbo;
			GL::EBO ebo;
			GL::FBO lightFBO;
			GL::Texture blurredLightTexture;
			std::vector<GLint> brightTiles;
			std::unordered_set<TileID> brightSet;
			RectangleRenderer rectangle;
			Reshader reshader;
			Realm &realm;
			std::vector<TileID> tileCache;

			void generateVertexBufferObject();
			void generateElementBufferObject();
			void generateVertexArrayObject();
			void generateLightingTexture();

			void recomputeLighting();
	};
}
