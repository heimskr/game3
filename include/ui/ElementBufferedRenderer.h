#pragma once

#include <unordered_set>
#include <vector>

#include "Shader.h"
#include "ui/RectangleRenderer.h"
#include "ui/Reshader.h"
#include "util/GL.h"

namespace Game3 {
	class ElementBufferedRenderer {
		public:
			constexpr static float tileTexturePadding = 1.f / 2048.f;
			float scale = 1.f;
			int backbufferWidth = -1;
			int backbufferHeight = -1;

			Eigen::Vector2f center {0.f, 0.f};
			TilemapPtr tilemap;
			GL::Texture lightTexture;

			ElementBufferedRenderer(Realm &);
			~ElementBufferedRenderer();

			void reset();
			void init(TilemapPtr);
			void render(float divisor);
			void reupload();
			bool onBackbufferResized(int width, int height);
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
			GL::FBO mainFBO;
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

			static void check(int handle, bool is_link = false);
	};
}
