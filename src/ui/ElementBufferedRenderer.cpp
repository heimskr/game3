#include <iostream>

#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "resources.h"
#include "Tileset.h"
#include "realm/Realm.h"
#include "ui/ElementBufferedRenderer.h"
#include "util/Timer.h"
#include "util/Util.h"

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/ElementBufferedRenderer.cs

namespace Game3 {
	ElementBufferedRenderer::ElementBufferedRenderer(Realm &realm_):
		reshader(blur_frag), realm(realm_) {}

	ElementBufferedRenderer::~ElementBufferedRenderer() {
		reset();
	}

	void ElementBufferedRenderer::reset() {
		if (initialized) {
			vao.reset();
			ebo.reset();
			vbo.reset();
			shader.reset();
			lightTexture.reset();
			blurredLightTexture.reset();
			mainFBO.reset();
			lightFBO.reset();
			reshader.reset();
			tilemap.reset();
			rectangle.reset();
			initialized = false;
		}
	}

	void ElementBufferedRenderer::init(TilemapPtr tilemap_) {
		if (initialized)
			reset();
		tilemap = std::move(tilemap_);
		shader.init(buffered_vert, buffered_frag);
		generateVertexBufferObject();
		generateElementBufferObject();
		generateVertexArrayObject();
		mainFBO.init();
		lightFBO.init();
		generateLightingTexture();
		const auto bright_shorts = tilemap->tileset->getBrightIDs();
		brightTiles.assign(bright_shorts.begin(), bright_shorts.end());
		brightTiles.resize(8, -1);
		brightSet = {bright_shorts.begin(), bright_shorts.end()};
		initialized = true;
	}

	void ElementBufferedRenderer::render(GLuint target_texture, float divisor) {
		if (!initialized)
			return;

		// GL::useTextureInFB(target_texture);
		tilemap->getTexture(realm.getGame())->bind(0);

		if (dirty) {
			recomputeLighting();
			dirty = false;
		}

		constexpr float center_scale = 0.f;
		constexpr float size_scale = -1.f;

		glm::mat4 projection(1.f);
		projection = //glm::scale(projection, {tilemap->tileSize, -tilemap->tileSize, 1.f}) *
		            //  glm::scale(projection, {scale / backbufferWidth, scale / backbufferHeight, 1.f}) *
		             glm::scale(projection, {32.f / backbufferWidth, 32.f / backbufferHeight, 1.f}) *
		             glm::translate(projection, {center.x() * center_scale + tilemap->width * size_scale, center.y() * center_scale + tilemap->height * size_scale, 0});
		            //  glm::translate(projection, {0 - tilemap->width / 2.f, 0 - tilemap->height / 2.f, 0.f});

		shader.bind();
		vao.bind();
		vbo.bind();
		ebo.bind();
		shader.set("texture0", 0);
		shader.set("projection", projection);
		shader.set("divisor", divisor);
		shader.set("bright_tiles", brightTiles);

		GL::triangles(tilemap->size());
	}

	void ElementBufferedRenderer::reupload() {
		generateVertexBufferObject();
		generateVertexArrayObject();
	}

	bool ElementBufferedRenderer::onBackbufferResized(int width, int height) {
		if (width == backbufferWidth && height == backbufferHeight)
			return false;
		backbufferWidth = width;
		backbufferHeight = height;
		generateLightingTexture();
		return true;
	}

	void ElementBufferedRenderer::generateVertexBufferObject() {
		const auto set_width = tilemap->setWidth / tilemap->tileSize;
		const float divisor = set_width;
		const float t_size = 1.f / divisor - tileTexturePadding * 2;

		vbo.init<float, 3>(tilemap->width, tilemap->height, GL_STATIC_DRAW, [this, set_width, divisor, t_size](size_t x, size_t y) {
			const auto tile = (*tilemap)(x, y);
			const float tx0 = (tile % set_width) / divisor + tileTexturePadding;
			const float ty0 = (tile / set_width) / divisor + tileTexturePadding;
			const float tile_f = static_cast<float>(tile);
			return std::array {
				std::array {tx0,          ty0,          tile_f},
				std::array {tx0 + t_size, ty0,          tile_f},
				std::array {tx0,          ty0 + t_size, tile_f},
				std::array {tx0 + t_size, ty0 + t_size, tile_f},
			};
		});
	}

	void ElementBufferedRenderer::generateElementBufferObject() {
		uint32_t i = 0;
		ebo.init<uint32_t, 6>(tilemap->width, tilemap->height, GL_STATIC_DRAW, [this, &i](size_t, size_t) {
			i += 4;
			return std::array {i - 4, i - 3, i - 2, i - 3, i - 2, i - 1};
		});
	}

	void ElementBufferedRenderer::generateVertexArrayObject() {
		assert(vbo.getHandle() != 0);
		vao.init(vbo, {2, 2, 1});
	}

	void ElementBufferedRenderer::generateLightingTexture() {
		const auto width  = tilemap->tileSize * tilemap->width;
		const auto height = tilemap->tileSize * tilemap->height;
		constexpr GLint filter = GL_NEAREST;

		lightTexture.initFloat(width, height, filter);
		blurredLightTexture.initFloat(width, height, filter);

		rectangle.update(width, height);
		reshader.update(width, height);
	}

	void ElementBufferedRenderer::recomputeLighting() {
		if (!tilemap)
			return;

		Timer::clear();
		Timer timer("RecomputeLighting");

		bool recomputation_needed = false;
		if (tileCache.empty()) {
			tileCache = tilemap->tiles;
			recomputation_needed = true;
		} else {
			assert(tileCache.size() == tilemap->size());
			for (size_t i = 0, max = tileCache.size(); i < max; ++i) {
				if (brightSet.contains(tileCache[i]) != brightSet.contains(tilemap->tiles[i])) {
					tileCache = tilemap->tiles;
					recomputation_needed = true;
					break;
				}
			}
		}

		if (recomputation_needed) {
			lightFBO.bind();
			generateLightingTexture();

			const auto tilesize = tilemap->tileSize;
			const auto width    = tilesize * tilemap->width;
			const auto height   = tilesize * tilemap->height;
			GL::Viewport viewport(0, 0, width, height);

			lightTexture.useInFB();

			GL::clear(.0f, .0f, .0f, 0.f);

			const TileID lava = (*tilemap->tileset)["base:tile/lava"];

			Timer lava_timer("Lava");
			for (Index row = 0; row < tilemap->height; ++row) {
				for (Index column = 0; column < tilemap->width; ++column) {
					const Position pos(row, column);
					const auto tile = (*tilemap)[pos];
					if (tile == lava) {
						const float x = column * tilesize;
						const float y = row * tilesize;
						constexpr float radius = 1.5f;
						rectangle({1.f, .5f, 0.f, .5f}, x - radius * tilesize, y - radius * tilesize, (2.f * radius + 1.f) * tilesize, (2.f * radius + 1.f) * tilesize);
					}
				}
			}
			lava_timer.stop();

			reshader.bind();
			reshader.set("xs", static_cast<float>(width));
			reshader.set("ys", static_cast<float>(height));
			reshader.set("r", 5.f);

			Timer blur("Blur");
			for (int i = 0; i < 8; ++i) {
				blurredLightTexture.useInFB();
				reshader.set("axis", 0);
				reshader(lightTexture);

				lightTexture.useInFB();
				reshader.set("axis", 1);
				reshader(blurredLightTexture);
			}
			blur.stop();

			viewport.reset();
			lightFBO.undo();
		}

		timer.stop();
		Timer::summary();
	}

	void ElementBufferedRenderer::check(int handle, bool is_link) {
		int success;
		char info[1024];
		if (is_link)
			glGetProgramiv(handle, GL_LINK_STATUS, &success);
		else
			glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLsizei len = 666;
			if (is_link)
				glGetProgramInfoLog(handle, GL_INFO_LOG_LENGTH, &len, info);
			else
				glGetShaderInfoLog(handle, 1024, &len, info);
			std::cerr << "Error with " << handle << " (l=" << len << "): " << info << '\n';
		}
	}
}
