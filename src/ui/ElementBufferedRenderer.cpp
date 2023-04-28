// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/ElementBufferedRenderer.cs

#include <iostream>

#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "resources.h"
#include "Tileset.h"
#include "container/Quadtree.h"
#include "realm/Realm.h"
#include "ui/ElementBufferedRenderer.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	ElementBufferedRenderer::ElementBufferedRenderer():
		reshader(blur_frag) {}

	ElementBufferedRenderer::ElementBufferedRenderer(Realm &realm_):
		reshader(blur_frag), realm(&realm_) {}

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
			reshader.reset();
			tilemap.reset();
			rectangle.reset();
			fbo.reset();
			initialized = false;
		}
	}

	void ElementBufferedRenderer::init(TileProvider &provider_) {
		if (initialized)
			reset();
		assert(realm);
		provider = &provider_;
		shader.init(buffered_vert, buffered_frag);
		generateVertexBufferObject();
		generateElementBufferObject();
		generateVertexArrayObject();
		generateLightingTexture();
		fbo.init();
		const auto bright_shorts = provider->getTileset(realm->getGame())->getBrightIDs();
		brightTiles.assign(bright_shorts.begin(), bright_shorts.end());
		brightTiles.resize(8, -1);
		brightSet = {bright_shorts.begin(), bright_shorts.end()};
		initialized = true;
	}

	void ElementBufferedRenderer::render(float divisor, float scale, float center_x, float center_y) {
		if (!initialized)
			return;

		assert(realm);

		if (dirty) {
			recomputeLighting();
			dirty = false;
		}

		glm::mat4 projection(1.f);
		projection = glm::scale(projection, {tilemap->tileSize, -tilemap->tileSize, 1.f}) *
		             glm::scale(projection, {scale / backbufferWidth, scale / backbufferHeight, 1.f}) *
		             glm::translate(projection, {center_x - tilemap->width / 2.f, center_y - tilemap->height / 2.f, 0.f});

		shader.bind();
		vao.bind();
		vbo.bind();
		ebo.bind();
		// Try commenting this out, it's kinda funny
		tilemap->getTexture(realm->getGame())->bind(0);
		shader.set("texture0", 0);
		shader.set("projection", projection);
		shader.set("divisor", divisor);
		shader.set("bright_tiles", brightTiles);

		GL::triangles(tilemap->size());
	}

	void ElementBufferedRenderer::render(float divisor) {
		if (!initialized)
			return;

		if (dirty) {
			recomputeLighting();
			dirty = false;
		}

		glm::mat4 projection(1.f);
		projection = glm::scale(projection, {tilemap->tileSize, tilemap->tileSize, 1.f}) *
		             glm::scale(projection, {2.f / backbufferWidth, 2.f / backbufferHeight, 1.f}) *
		             glm::translate(projection, {-tilemap->width, -tilemap->height, 0.f});

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
		return true;
	}

	void ElementBufferedRenderer::setChunk(Chunk<TileID> &new_chunk, bool can_reupload) {
		if (&new_chunk == chunk)
			return;
		chunk = &new_chunk;
		if (can_reupload)
			reupload();
	}

	void ElementBufferedRenderer::generateVertexBufferObject() {
		const auto set_width = tilemap->setWidth / tilemap->tileSize;
		const float divisor = set_width;
		const float t_size = 1.f / divisor - TILE_TEXTURE_PADDING * 2;

		vbo.init<float, 3>(tilemap->width, tilemap->height, GL_STATIC_DRAW, [this, set_width, divisor, t_size](size_t x, size_t y) {
			const auto tile = (*tilemap)(x, y);
			const float tx0 = (tile % set_width) / divisor + TILE_TEXTURE_PADDING;
			const float ty0 = (tile / set_width) / divisor + TILE_TEXTURE_PADDING;
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
		ebo.init<uint32_t, 6>(tilemap->width, tilemap->height, GL_STATIC_DRAW, [&i](size_t, size_t) {
			i += 4;
			return std::array {i - 4, i - 3, i - 2, i - 3, i - 2, i - 1};
		});
	}

	void ElementBufferedRenderer::generateVertexArrayObject() {
		assert(vbo.getHandle() != 0);
		vao.init(vbo, {2, 2, 1});
	}

	void ElementBufferedRenderer::generateLightingTexture() {}

	void ElementBufferedRenderer::recomputeLighting() {
		return;

		if (!tilemap)
			return;

		Timer timer("RecomputeLighting");

		bool recomputation_needed = false;
		if (tileCache.empty()) {
			tileCache = tilemap->getTiles();
			recomputation_needed = true;
		} else {
			assert(tileCache.size() == tilemap->size());
			for (size_t i = 0, max = tileCache.size(); i < max; ++i) {
				if (brightSet.contains(tileCache[i]) != brightSet.contains((*tilemap)[i])) {
					tileCache = tilemap->getTiles();
					recomputation_needed = true;
					break;
				}
			}
		}

		if (recomputation_needed) {
			fbo.bind();
			constexpr float texture_scale = 2.f;
			constexpr GLint filter = GL_LINEAR;
			const auto tilesize = tilemap->tileSize;
			const auto width  = tilesize * texture_scale * tilemap->width;
			const auto height = tilesize * texture_scale * tilemap->height;
			rectangle.update(width, height);
			reshader.update(width, height);
			GL::Viewport viewport(0, 0, width, height);
			lightTexture.initFloat(width, height, filter);
			blurredLightTexture.initFloat(width, height, filter);
			lightTexture.useInFB();
			GL::clear(.5f, .5f, .5f, 1.f);

			if (tilemap->lavaQuadtree) {
				Timer lava_timer("Lava");
				tilemap->lavaQuadtree->absorb();
				tilemap->lavaQuadtree->iterateFull([&](const Box &box) {
					const float x = box.left * tilesize;
					const float y = box.top  * tilesize;
					constexpr float bleed = 1.5f;
					rectangle({1.f, .5f, 0.f, 1.f}, x - bleed * tilesize, y - bleed * tilesize, (2.f * bleed + box.width) * tilesize, (2.f * bleed + box.height) * tilesize);
					return false;
				});
			}

			reshader.bind();
			reshader.set("xs", static_cast<float>(width));
			reshader.set("ys", static_cast<float>(height));
			reshader.set("r", 5.f);

			blurredLightTexture.useInFB();

			Timer blur("Blur");
			for (int i = 0; i < 8; ++i) {
				if (i != 0)
					blurredLightTexture.useInFB();
				reshader.set("axis", 0);
				reshader(lightTexture);

				lightTexture.useInFB();
				reshader.set("axis", 1);
				reshader(blurredLightTexture);
			}
			blur.stop();

			viewport.reset();
			GL::unbindFBTexture();
			fbo.undo();
		}

		timer.stop();
	}

	void ElementBufferedRenderer::check(int handle, bool is_link) {
		int success;
		std::array<char, 1024> info {};
		if (is_link)
			glGetProgramiv(handle, GL_LINK_STATUS, &success);
		else
			glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLsizei len = 666;
			if (is_link)
				glGetProgramInfoLog(handle, GL_INFO_LOG_LENGTH, &len, info.data());
			else
				glGetShaderInfoLog(handle, 1024, &len, info.data());
			std::cerr << "Error with " << handle << " (l=" << len << "): " << info.data() << '\n';
		}
	}
}
