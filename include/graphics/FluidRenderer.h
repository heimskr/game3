#pragma once

#include "game/TileProvider.h"
#include "graphics/GL.h"
#include "graphics/RectangleRenderer.h"
#include "graphics/Shader.h"
#include "math/Vector.h"
#include "threading/Lockable.h"
#include "types/Types.h"

#include <atomic>
#include <unordered_set>
#include <vector>

namespace Game3 {
	class Realm;

	class FluidRenderer {
		public:
			constexpr static float TEXTURE_SCALE = 2.f;
			constexpr static float TILE_TEXTURE_PADDING = 1.f / 2048.f;
			std::atomic_int backbufferWidth = -1;
			std::atomic_int backbufferHeight = -1;
			std::atomic_bool isMissing = false;

			Vector2d center{0, 0};

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
			std::atomic_bool initialized = false;
			Shader shader {"fluids"};
			GL::FloatVAO vao;
			GL::VBO vbo;
			GL::EBO ebo;
			GL::FBO fbo;
			Realm *realm = nullptr;
			FluidChunk *chunk = nullptr;
			TileProvider *provider = nullptr;
			std::atomic_bool positionDirty = false;
			Lockable<ChunkPosition> chunkPosition;

			bool generateVertexBufferObject();
			bool generateElementBufferObject();
			bool generateVertexArrayObject();
	};
}
