#pragma once

#include <memory>

#include <Eigen/Eigen>

#include "Tilemap.h"

namespace Game3 {
	struct TileSet;

	class TilemapRenderer {
		public:
			constexpr static float tileTextureSize = 1 / 10.f;
			constexpr static float tileTexturePadding = 1 / 2048.f;
			float scale = 2.f;
			int backbufferWidth = -1;
			int backbufferHeight = -1;

			Eigen::Vector2f center {0.f, 0.f};
			TilemapPtr tilemap;

			TilemapRenderer() = default;
			virtual ~TilemapRenderer() = default;

			virtual void init(TilemapPtr, const TileSet &) = 0;
			virtual void render(float divisor) = 0;
			virtual bool onBackbufferResized(int width, int height);

		protected:
			static void check(int handle, bool is_link = false);
	};
}
