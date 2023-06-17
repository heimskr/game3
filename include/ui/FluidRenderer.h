#pragma once

#include <atomic>
#include <unordered_set>
#include <vector>

#include "Shader.h"
#include "Types.h"
#include "game/TileProvider.h"
#include "ui/RectangleRenderer.h"
#include "util/GL.h"

namespace Game3 {
	class Realm;

	class FluidRenderer {
		public:
			constexpr static float TEXTURE_SCALE = 2.f;
			constexpr static float TILE_TEXTURE_PADDING = 1.f / 2048.f;
			int backbufferWidth = -1;
			int backbufferHeight = -1;
			bool isMissing = false;

			Eigen::Vector2f center {0.f, 0.f};

			FluidRenderer();
			FluidRenderer(Realm &);
			~FluidRenderer();

			void reset();
			void init();
			void setup(TileProvider &);
			void render(float divisor, float scale, float center_x, float center_y);
			bool reupload();
			bool onBackbufferResized(int width, int height);
			void setChunk(FluidChunk &, bool can_reupload = true);
			void setChunkPosition(const ChunkPosition &);
			inline void setRealm(Realm &new_realm) { realm = &new_realm; }

			void snooze();
			void wakeUp();

			inline explicit operator bool() const { return initialized; }

		private:
			bool initialized = false;
			Shader shader {"fluids"};
			GL::FloatVAO vao;
			GL::VBO vbo;
			GL::EBO ebo;
			GL::FBO fbo;
			Realm *realm = nullptr;
			FluidChunk *chunk = nullptr;
			TileProvider *provider = nullptr;
			bool positionDirty = false;
			ChunkPosition chunkPosition;

			bool generateVertexBufferObject();
			bool generateElementBufferObject();
			bool generateVertexArrayObject();
	};
}
