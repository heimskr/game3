// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/ElementBufferedRenderer.cs

#include <iostream>

#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Log.h"
#include "resources.h"
#include "Tileset.h"
#include "container/Quadtree.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "ui/FluidRenderer.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	FluidRenderer::FluidRenderer() = default;

	FluidRenderer::FluidRenderer(Realm &realm_):
		realm(&realm_) {}

	FluidRenderer::~FluidRenderer() {
		reset();
	}

	void FluidRenderer::reset() {
		if (initialized) {
			vao.reset();
			ebo.reset();
			vbo.reset();
			shader.reset();
			fbo.reset();
			initialized = false;
		}
	}

	void FluidRenderer::init() {
		if (initialized)
			reset();
		assert(realm);
		shader.init(fluids_vert, fluids_frag);
		generateVertexBufferObject();
		generateElementBufferObject();
		generateVertexArrayObject();
		initialized = true;
	}

	void FluidRenderer::setup(TileProvider &provider_) {
		provider = &provider_;
	}

	void FluidRenderer::render(float divisor, float scale, float center_x, float center_y) {
		if (!initialized)
			return;

		assert(realm);

		if (vbo.getHandle() == 0 || positionDirty) {
			if (!reupload())
				return;
			positionDirty = false;
		}

		auto &tileset = realm->getTileset();
		const auto tilesize = static_cast<float>(tileset.getTileSize());
		auto texture = tileset.getTexture(realm->getGame());

		glm::mat4 projection(1.f);
		projection = glm::scale(projection, {tilesize, -tilesize, 1.f}) *
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
		texture->bind(0);
		shader.set("texture0", 0);
		shader.set("projection", projection);
		shader.set("divisor", divisor);

		GL::triangles(CHUNK_SIZE * CHUNK_SIZE);
	}

	bool FluidRenderer::reupload() {
		return generateVertexBufferObject() && generateVertexArrayObject();
	}

	bool FluidRenderer::onBackbufferResized(int width, int height) {
		if (width == backbufferWidth && height == backbufferHeight)
			return false;
		backbufferWidth = width;
		backbufferHeight = height;
		return true;
	}

	void FluidRenderer::setChunk(TileProvider::FluidChunk &new_chunk, bool can_reupload) {
		if (&new_chunk == chunk)
			return;
		chunk = &new_chunk;
		if (can_reupload)
			reupload();
	}

	void FluidRenderer::setChunkPosition(const ChunkPosition &new_pos) {
		if (new_pos != chunkPosition) {
			chunkPosition = new_pos;
			positionDirty = true;
		}
	}

	void FluidRenderer::snooze() {
		reset();
	}

	void FluidRenderer::wakeUp() {
		init();
	}

	bool FluidRenderer::generateVertexBufferObject() {
		assert(realm);

		auto &game = realm->getGame();
		auto &tileset = realm->getTileset();
		const auto tilesize = tileset.getTileSize();
		const auto tileset_width = *tileset.getTexture(realm->getGame())->width;

		const auto set_width = tileset_width / tilesize;

		if (set_width == 0)
			return false;

		const float divisor = set_width;
		const float t_size = 1.f / divisor - TILE_TEXTURE_PADDING * 2;

		isMissing = false;

		const TileID missing = tileset["base:tile/missing"];

		vbo.init<float, 4>(CHUNK_SIZE, CHUNK_SIZE, GL_STATIC_DRAW, [this, &game, set_width, divisor, t_size, missing](size_t x, size_t y) {
			const auto fluid_opt = realm->tileProvider.copyFluidTile({
				static_cast<Index>(y) + CHUNK_SIZE * (chunkPosition.y + 1), // why `+ 1`?
				static_cast<Index>(x) + CHUNK_SIZE * (chunkPosition.x + 1)  // here too
			});

			TileID tile = -1;
			float opacity;

			if (fluid_opt) {
				if (auto tile_opt = game.getFluidTileID(fluid_opt->id)) {
					tile = *tile_opt;
					if (fluid_opt->level == UINT16_MAX)
						opacity = 1.f;
					else
						opacity = fluid_opt->level / 65534.f;
				}
			}

			if (tile == static_cast<uint16_t>(-1)) {
				isMissing = true;
				tile = missing;
				opacity	= 0.f;
			}

			const float tx0 = (tile % set_width) / divisor + TILE_TEXTURE_PADDING;
			const float ty0 = (tile / set_width) / divisor + TILE_TEXTURE_PADDING;
			const float tile_f = static_cast<float>(tile);
			return std::array {
				std::array {tx0,          ty0,          tile_f, opacity},
				std::array {tx0 + t_size, ty0,          tile_f, opacity},
				std::array {tx0,          ty0 + t_size, tile_f, opacity},
				std::array {tx0 + t_size, ty0 + t_size, tile_f, opacity},
			};
		});

		return vbo.getHandle() != 0;
	}

	bool FluidRenderer::generateElementBufferObject() {
		uint32_t i = 0;
		ebo.init<uint32_t, 6>(CHUNK_SIZE, CHUNK_SIZE, GL_STATIC_DRAW, [&i](size_t, size_t) {
			i += 4;
			return std::array {i - 4, i - 3, i - 2, i - 3, i - 2, i - 1};
		});
		return ebo.getHandle() != 0;
	}

	bool FluidRenderer::generateVertexArrayObject() {
		if (vbo.getHandle() != 0)
			vao.init(vbo, {2, 2, 1, 1});
		return vao.getHandle() != 0;
	}
}
