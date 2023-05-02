// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/ElementBufferedRenderer.cs

#include <iostream>

#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "resources.h"
#include "Tileset.h"
#include "container/Quadtree.h"
#include "game/Game.h"
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
			tileset.reset();
			rectangle.reset();
			fbo.reset();
			initialized = false;
		}
	}

	void ElementBufferedRenderer::init(TileProvider &provider_, Layer layer_) {
		if (initialized)
			reset();
		assert(realm);
		layer = layer_;
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

		if (vbo.getHandle() == 0 || positionDirty) {
			if (!reupload())
				return;
			positionDirty = false;
		}

		if (dirty) {
			recomputeLighting();
			dirty = false;
		}

		auto &tileset = realm->getTileset();
		const auto tilesize = tileset.getTileSize();
		auto texture = tileset.getTexture(realm->getGame());

		glm::mat4 projection(1.f);
		projection = glm::scale(projection, {static_cast<float>(tilesize), -static_cast<float>(tilesize), 1.f}) *
		             glm::scale(projection, {scale / backbufferWidth, scale / backbufferHeight, 1.f}) *
		             glm::translate(projection, {
		                 center_x - CHUNK_SIZE / 2.f + chunkPosition.x * CHUNK_SIZE,
		                 center_y - CHUNK_SIZE / 2.f + chunkPosition.y * CHUNK_SIZE,
		                 0.f
		             });

		shader.bind();
		vao.bind();
		vbo.bind();
		ebo.bind();
		// Try commenting this out, it's kinda funny
		texture->bind(0);
		shader.set("texture0", 0);
		shader.set("projection", projection);
		shader.set("divisor", divisor);
		shader.set("bright_tiles", brightTiles);

		GL::triangles(CHUNK_SIZE * CHUNK_SIZE);
	}

	void ElementBufferedRenderer::render(float divisor) {
		if (!initialized)
			return;

		assert(realm);

		if (vbo.getHandle() == 0 || positionDirty) {
			if (!reupload())
				return;
			positionDirty = false;
		}

		if (dirty) {
			recomputeLighting();
			dirty = false;
		}

		auto &tileset = realm->getTileset();
		const auto tilesize = tileset.getTileSize();

		glm::mat4 projection(1.f);
		projection = glm::scale(projection, {tilesize, tilesize, 1.f}) *
		             glm::scale(projection, {2.f / backbufferWidth, 2.f / backbufferHeight, 1.f}) *
		             glm::translate(projection, {-CHUNK_SIZE, -CHUNK_SIZE, 0.f});

		shader.bind();
		vao.bind();
		vbo.bind();
		ebo.bind();
		tileset.getTexture(realm->getGame())->bind(0);
		shader.set("texture0", 0);
		shader.set("projection", projection);
		shader.set("divisor", divisor);
		shader.set("bright_tiles", brightTiles);

		GL::triangles(CHUNK_SIZE * CHUNK_SIZE);
	}

	bool ElementBufferedRenderer::reupload() {
		return generateVertexBufferObject() && generateVertexArrayObject();
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

	void ElementBufferedRenderer::setChunkPosition(const ChunkPosition &new_pos) {
		if (new_pos != chunkPosition) {
			chunkPosition = new_pos;
			positionDirty = true;
		}
	}

	bool ElementBufferedRenderer::generateVertexBufferObject() {
		assert(realm);

		auto &tileset = realm->getTileset();
		const auto tilesize = tileset.getTileSize();
		const auto tileset_width = *tileset.getTexture(realm->getGame())->width;

		const auto set_width = tileset_width / tilesize;

		if (set_width == 0)
			return false;

		const float divisor = set_width;
		const float t_size = 1.f / divisor - TILE_TEXTURE_PADDING * 2;

		vboIsWeird = false;

		const TileID missing = tileset[tileset.getMissing()];

		vbo.init<float, 3>(CHUNK_SIZE, CHUNK_SIZE, GL_STATIC_DRAW, [this, set_width, divisor, t_size, missing](size_t x, size_t y) {
			const auto tile_opt = realm->tryTile(layer, Position(
				static_cast<Index>(y) + CHUNK_SIZE * chunkPosition.y,
				static_cast<Index>(x) + CHUNK_SIZE * chunkPosition.x
			));
			TileID tile;
			if (!tile_opt) {
				vboIsWeird = true;
				tile = missing;
			} else
				tile = *tile_opt;
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

		return vbo.getHandle() != 0;
	}

	bool ElementBufferedRenderer::generateElementBufferObject() {
		uint32_t i = 0;
		ebo.init<uint32_t, 6>(CHUNK_SIZE, CHUNK_SIZE, GL_STATIC_DRAW, [&i](size_t, size_t) {
			i += 4;
			return std::array {i - 4, i - 3, i - 2, i - 3, i - 2, i - 1};
		});
		return ebo.getHandle() != 0;
	}

	bool ElementBufferedRenderer::generateVertexArrayObject() {
		if (vbo.getHandle() != 0)
			vao.init(vbo, {2, 2, 1});
		return vao.getHandle() != 0;
	}

	bool ElementBufferedRenderer::generateLightingTexture() {
		return true;
	}

	void ElementBufferedRenderer::recomputeLighting() {
		return;

		Timer timer("RecomputeLighting");

		bool recomputation_needed = false;
		if (tileCache.empty()) {
			tileCache = realm->tileProvider.getTileChunk(layer, chunkPosition);
			recomputation_needed = true;
		} else {
			assert(tileCache.size() == CHUNK_SIZE * CHUNK_SIZE);
			const auto &chunk = realm->tileProvider.getTileChunk(layer, chunkPosition);
			for (size_t i = 0, max = tileCache.size(); i < max; ++i) {
				if (brightSet.contains(tileCache[i]) != brightSet.contains(chunk[i])) {
					tileCache = chunk;
					recomputation_needed = true;
					break;
				}
			}
		}

		if (recomputation_needed) {
			fbo.bind();
			constexpr float texture_scale = 2.f;
			constexpr GLint filter = GL_LINEAR;
			const auto tilesize = tileset->getTileSize();
			const auto width  = tilesize * texture_scale * CHUNK_SIZE;
			const auto height = tilesize * texture_scale * CHUNK_SIZE;
			rectangle.update(width, height);
			reshader.update(width, height);
			GL::Viewport viewport(0, 0, width, height);
			lightTexture.initFloat(width, height, filter);
			blurredLightTexture.initFloat(width, height, filter);
			lightTexture.useInFB();
			GL::clear(.5f, .5f, .5f, 1.f);

			// TODO: lava quadtrees in TileProvider

			// if (tilemap->lavaQuadtree) {
			// 	Timer lava_timer("Lava");
			// 	tilemap->lavaQuadtree->absorb();
			// 	tilemap->lavaQuadtree->iterateFull([&](const Box &box) {
			// 		const float x = box.left * tilesize;
			// 		const float y = box.top  * tilesize;
			// 		constexpr float bleed = 1.5f;
			// 		rectangle({1.f, .5f, 0.f, 1.f}, x - bleed * tilesize, y - bleed * tilesize, (2.f * bleed + box.width) * tilesize, (2.f * bleed + box.height) * tilesize);
			// 		return false;
			// 	});
			// }

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
