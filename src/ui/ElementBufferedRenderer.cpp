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
	ElementBufferedRenderer::ElementBufferedRenderer(): reshader(blur_frag) {}

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
			lightFBO.reset();
			tilemap.reset();
			initialized = false;
		}
	}

	void ElementBufferedRenderer::init(TilemapPtr tilemap_, const Tileset &tileset) {
		if (initialized)
			reset();
		tilemap = std::move(tilemap_);
		shader.init(buffered_vert, buffered_frag);
		generateVertexBufferObject();
		generateElementBufferObject();
		generateVertexArrayObject();
		lightFBO.init();
		generateLightingTexture();
		const auto bright_shorts = tileset.getBrightIDs();
		brightTiles.assign(bright_shorts.begin(), bright_shorts.end());
		brightTiles.resize(8, -1);
		brightSet = {bright_shorts.begin(), bright_shorts.end()};
		initialized = true;
	}

	void ElementBufferedRenderer::render(float divisor) {
		if (!initialized)
			return;

		tilemap->texture->bind();

		if (dirty) {
			recomputeLighting();
			dirty = false;
		}

		lightTexture.bind(1);
		vao.bind();
		ebo.bind();

		glm::mat4 projection(1.f);
		projection = glm::scale(projection, {tilemap->tileSize, -tilemap->tileSize, 1}) *
		             glm::scale(projection, {scale / backbufferWidth, scale / backbufferHeight, 1}) *
		             glm::translate(projection, {center.x() - tilemap->width / 2.f, center.y() - tilemap->height / 2.f, 0});

		shader.bind();
		shader.set("texture0", 0);
		shader.set("texture1", 1);
		shader.set("projection", projection);
		shader.set("divisor", divisor);
		shader.set("bright_tiles", brightTiles);
		shader.set("map_size", static_cast<GLfloat>(tilemap->width), static_cast<GLfloat>(tilemap->height));

		GL::triangles(tilemap->size());
	}

	void ElementBufferedRenderer::reupload() {
		generateVertexBufferObject();
		generateVertexArrayObject();
	}

	bool ElementBufferedRenderer::onBackbufferResized(int width, int height) {
		if (TilemapRenderer::onBackbufferResized(width, height)) {
			generateLightingTexture();
			return true;
		}
		return false;
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
			const auto gtk_buffer = GL::getFB();

			lightFBO.bind();
			generateLightingTexture();

			const auto tilesize = tilemap->tileSize;
			const auto width    = tilesize * tilemap->width;
			const auto height   = tilesize * tilemap->height;
			GL::Viewport viewport(0, 0, width, height);

			lightTexture.useInFB();

			// Clearing to half-white because the color in the lightmap will be multiplied by two
			GL::clear(.5f, .5f, .5f, 0.f);

			const TileID lava = (*tilemap->tileset)["base:tile/lava"];

			Timer lava1("Lava1");
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
			lava1.stop();

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

			Timer lava2("Lava2");
			for (Index row = 0; row < tilemap->height; ++row) {
				for (Index column = 0; column < tilemap->width; ++column) {
					const Position pos(row, column);
					const auto tile = (*tilemap)[pos];
					if (tile == lava) {
						const float x = column * tilesize;
						const float y = row * tilesize;
						const float margin = .0f * tilesize;
						rectangle({1.f, 1.f, 1.f, 1.f}, x + margin, y + margin, tilesize - 2 * margin, tilesize - 2 * margin);
					}
				}
			}
			lava2.stop();

			viewport.reset();
			GL::bindFB(gtk_buffer);
		}

		timer.stop();
		Timer::summary();
	}
}
