#pragma once

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/GeometryRenderer.cs

#include <memory>

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>

#include "Tilemap.h"

namespace Game3 {
	class TilemapRenderer {
		public:
			constexpr static float tileTextureSize = 1 / 16.f;
			constexpr static float tileTexturePadding = 1 / 256.f;

			nanogui::Vector2f center {0.f, 0.f};
			std::shared_ptr<Tilemap> tilemap;

			TilemapRenderer() = default;
			~TilemapRenderer();

			void initialize(const std::shared_ptr<Tilemap> &);
			void render();
			void onBackBufferResized(int width, int height);

		private:
			GLuint shaderHandle = -1;
			GLuint vboHandle = -1;
			GLuint vaoHandle = -1;
			int backBufferWidth = -1;
			int backBufferHeight = -1;

			void createShader();
			void generateVertexBufferObject();
			void generateVertexArrayObject();
	};
}
