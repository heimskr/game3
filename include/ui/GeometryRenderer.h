#pragma once

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/GeometryRenderer.cs

#include <GL/gl.h>
#include <GL/glext.h>

#include "ui/TilemapRenderer.h"

namespace Game3 {
	class GeometryRenderer: public TilemapRenderer {
		public:
			constexpr static float tileTextureSize = 1 / 16.f;
			constexpr static float tileTexturePadding = 1 / 256.f;
			float scale = 4.f;

			nanogui::Vector2f center {0.f, 0.f};
			std::shared_ptr<Tilemap> tilemap;

			GeometryRenderer() = default;
			~GeometryRenderer() override;

			void initialize(const std::shared_ptr<Tilemap> &) override;
			void render() override;

		private:
			GLuint shaderHandle = -1;
			GLuint vboHandle = -1;
			GLuint vaoHandle = -1;

			void createShader();
			void generateVertexBufferObject();
			void generateVertexArrayObject();
	};
}
