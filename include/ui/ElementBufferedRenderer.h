#pragma once

#include <unordered_set>
#include <vector>

#include "Shader.h"
#include "Types.h"
#include "ui/RectangleRenderer.h"
#include "ui/Reshader.h"
#include "util/GL.h"

namespace Game3 {
	class Realm;
	class Tilemap;

	/** Start corresponds to left or top, End corresponds to right or bottom. */
	enum class Alignment {Start, Middle, End};

	class ElementBufferedRenderer {
		public:
			constexpr static float TEXTURE_SCALE = 2.f;
			constexpr static float TILE_TEXTURE_PADDING = 1.f / 2048.f;
			int backbufferWidth = -1;
			int backbufferHeight = -1;
			Alignment horizontal;
			Alignment vertical;

			Eigen::Vector2f center {0.f, 0.f};
			std::shared_ptr<Tilemap> tilemap;
			GL::Texture lightTexture;

			ElementBufferedRenderer(Realm &);
			~ElementBufferedRenderer();

			void reset();
			void init(std::shared_ptr<Tilemap>);
			void render(float divisor, float scale, float center_x, float center_y);
			void render(float divisor);
			void reupload();
			bool onBackbufferResized(int width, int height);
			inline void markDirty() { dirty = true; }

			inline explicit operator bool() const { return initialized; }

		private:
			bool initialized = false;
			/** Whether lighting needs to be recomputed. */
			bool dirty = true;
			Shader shader {"terrain"};
			GL::FloatVAO vao;
			GL::VBO vbo;
			GL::EBO ebo;
			GL::FBO fbo;
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
