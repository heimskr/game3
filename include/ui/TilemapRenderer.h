#pragma once

#include <memory>

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>

#include "Tilemap.h"

namespace Game3 {
	class TilemapRenderer {
		public:
			constexpr static float tileTextureSize = 1 / 10.f;
			constexpr static float tileTexturePadding = 1 / 256.f;
			float scale = 4.f;

			nanogui::Vector2f center {0.f, 0.f};
			std::shared_ptr<Tilemap> tilemap;

			TilemapRenderer() = default;
			virtual ~TilemapRenderer() = default;

			virtual void initialize(const std::shared_ptr<Tilemap> &) = 0;
			virtual void render(NVGcontext *, int font) = 0;
			virtual void onBackBufferResized(int width, int height);

		protected:
			int backBufferWidth = -1;
			int backBufferHeight = -1;

			static void check(int handle, bool is_link = false);
	};
}
