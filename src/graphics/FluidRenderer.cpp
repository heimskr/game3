// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/ElementBufferedRenderer.cs

#include <iostream>

#include "graphics/Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Log.h"
#include "graphics/Tileset.h"
#include "container/Quadtree.h"
#include "game/Game.h"
#include "graphics/FluidRenderer.h"
#include "realm/Realm.h"
#include "util/FS.h"
#include "util/Timer.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		const std::string & fluidsFrag() { static auto out = readFile("resources/fluids.frag"); return out; }
		const std::string & fluidsVert() { static auto out = readFile("resources/fluids.vert"); return out; }
	}

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
		shader.init(fluidsVert(), fluidsFrag());
		generateVertexBufferObject();
		generateElementBufferObject();
		generateVertexArrayObject();
		initialized = true;
	}

	void FluidRenderer::setup(TileProvider &provider_) {
		provider = &provider_;
	}

	void FluidRenderer::render(double /* divisor */, double scale, double center_x, double center_y) {
		if (!initialized)
			return;

		assert(realm);

		if (vbo.getHandle() == 0 || positionDirty) {
			if (!reupload())
				return;
			positionDirty = false;
		}

		auto &tileset = realm->getTileset();
		const double tilesize(tileset.getTileSize());
		auto texture = tileset.getTexture(realm->getGame());
		const auto [chunk_x, chunk_y] = chunkPosition.copyBase();

		glm::dmat4 projection(1.);
		projection = glm::scale(projection, {tilesize, -tilesize, 1.}) *
		             glm::scale(projection, {scale / backbufferWidth, scale / backbufferHeight, 1.}) *
		             glm::translate(projection, {
		                 center_x - CHUNK_SIZE / 2. + chunk_x * CHUNK_SIZE,
		                 center_y - CHUNK_SIZE / 2. + chunk_y * CHUNK_SIZE,
		                 0.
		             });

		shader.bind();
		vao.bind();
		vbo.bind();
		ebo.bind();
		texture->bind(0);
		shader.set("texture0", 0);
		shader.set("projection", projection);
		// shader.set("divisor", divisor);

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

	void FluidRenderer::setChunk(FluidChunk &new_chunk, bool can_reupload) {
		if (&new_chunk == chunk)
			return;
		chunk = &new_chunk;
		if (can_reupload) {
			Timer timer{"FR::setChunk::reupload"};
			reupload();
		}
	}

	void FluidRenderer::setChunkPosition(const ChunkPosition &new_pos) {
		auto lock = chunkPosition.uniqueLock();
		if (new_pos != chunkPosition) {
			chunkPosition.getBase() = new_pos;
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
		const auto tileset_width = tileset.getTexture(realm->getGame())->width;

		const auto set_width = tileset_width / tilesize;

		if (set_width == 0)
			return false;

		const double divisor = set_width;
		const double t_size = 1. / divisor - TILE_TEXTURE_PADDING * 2;

		isMissing = false;
		const TileID missing = tileset["base:tile/missing"];

		const auto [chunk_x, chunk_y] = chunkPosition.copyBase();

		Timer timer{"FluidVBOInit"};
		vbo.init<double, 4>(CHUNK_SIZE, CHUNK_SIZE, GL_DYNAMIC_DRAW, [this, chunk_x = chunk_x, chunk_y = chunk_y, &game, set_width, divisor, t_size, missing](size_t x, size_t y) {
			const auto fluid_opt = realm->tileProvider.copyFluidTile({
				Index(y) + CHUNK_SIZE * (chunk_y + 1), // why `+ 1`?
				Index(x) + CHUNK_SIZE * (chunk_x + 1)  // here too
			});

			TileID tile = -1;
			double opacity;

			if (fluid_opt) {
				if (auto tile_opt = game.getFluidTileID(fluid_opt->id)) {
					tile = *tile_opt;
					if (FluidTile::FULL <= fluid_opt->level)
						opacity = 1.;
					else
						opacity = double(fluid_opt->level) / FluidTile::FULL;
				}
			}

			if (tile == static_cast<TileID>(-1)) {
				isMissing = true;
				tile = missing;
				opacity = 0.;
			}

			const double tx0 = (tile % set_width) / divisor + TILE_TEXTURE_PADDING;
			const double ty0 = (tile / set_width) / divisor + TILE_TEXTURE_PADDING;
			const double tile_f(tile);
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
