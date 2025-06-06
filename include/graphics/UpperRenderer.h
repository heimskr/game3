#pragma once

#include "graphics/Shader.h"
#include "types/Types.h"
#include "game/TileProvider.h"
#include "graphics/GL.h"
#include "threading/Lockable.h"

#include <atomic>
#include <future>
#include <unordered_set>
#include <vector>

namespace Game3 {
	class Realm;

	class UpperRenderer {
		public:
			int backbufferWidth = -1;
			int backbufferHeight = -1;

			Vector2d center{0, 0};
			std::shared_ptr<Tileset> tileset{};

			UpperRenderer();
			UpperRenderer(Realm &);
			~UpperRenderer();

			void reset();
			void init();
			void setup(TileProvider &);
			void render(float divisor, float scale, float center_x, float center_y);
			void render(float divisor);
			bool reupload();
			std::future<bool> queueReupload();
			bool onBackbufferResized(int width, int height);
			void setChunk(TileChunk &, bool can_reupload = true);
			void setChunkPosition(const ChunkPosition &);
			inline void setRealm(Realm &new_realm) { realm = &new_realm; }

			void snooze();
			void wakeUp();

			inline explicit operator bool() const { return initialized; }

		private:
			bool initialized = false;
			Shader shader{"terrain"};
			GL::FloatVAO vao{};
			GL::VBO vbo{};
			GL::EBO ebo{};
			GL::FBO fbo{};
			Realm *realm = nullptr;
			TileChunk *chunk = nullptr;
			TileProvider *provider = nullptr;
			std::vector<TileID> tileCache{};
			std::atomic_bool positionDirty = false;
			Lockable<ChunkPosition> chunkPosition{};

			bool generateVertexBufferObject();
			bool generateElementBufferObject();
			bool generateVertexArrayObject();

			static void check(int handle, bool is_link = false);
	};
}
