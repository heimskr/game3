#pragma once

#include "Shader.h"
#include "Types.h"
#include "game/TileProvider.h"
#include "ui/RectangleRenderer.h"
#include "ui/Reshader.h"
#include "util/GL.h"

#include <atomic>
#include <future>
#include <unordered_set>
#include <vector>

namespace Game3 {
	class Realm;

	class ElementBufferedRenderer {
		public:
			constexpr static float TEXTURE_SCALE = 2.f;
			constexpr static float TILE_TEXTURE_PADDING = 1.f / 2048.f;
			int backbufferWidth = -1;
			int backbufferHeight = -1;
			std::atomic_bool isMissing = false;

			Eigen::Vector2f center {0.f, 0.f};
			std::shared_ptr<Tileset> tileset;
			GL::Texture lightTexture;

			ElementBufferedRenderer();
			ElementBufferedRenderer(Realm &);
			~ElementBufferedRenderer();

			void reset();
			void init();
			void setup(TileProvider &, Layer);
			void render(float divisor, float scale, float center_x, float center_y);
			void render(float divisor);
			bool reupload();
			std::future<bool> queueReupload();
			bool onBackbufferResized(int width, int height);
			void setChunk(TileChunk &, bool can_reupload = true);
			void setChunkPosition(const ChunkPosition &);
			inline void markDirty() { dirty = true; }
			inline void setRealm(Realm &new_realm) { realm = &new_realm; }

			void snooze();
			void wakeUp();

			inline explicit operator bool() const { return initialized; }

		private:
			bool initialized = false;
			/** Whether lighting needs to be recomputed. */
			std::atomic_bool dirty = true;
			Layer layer = Layer::Invalid;
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
			Realm *realm = nullptr;
			TileChunk *chunk = nullptr;
			TileProvider *provider = nullptr;
			std::vector<TileID> tileCache;
			bool positionDirty = false;
			ChunkPosition chunkPosition;

			bool generateVertexBufferObject();
			bool generateElementBufferObject();
			bool generateVertexArrayObject();
			bool generateLightingTexture();

			void recomputeLighting();

			static void check(int handle, bool is_link = false);
	};
}
