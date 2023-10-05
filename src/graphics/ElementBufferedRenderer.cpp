// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/ElementBufferedRenderer.cs

#include "resources.h"
#include "graphics/Shader.h"
#include "graphics/Tileset.h"
#include "container/Quadtree.h"
#include "game/ClientGame.h"
#include "game/Game.h"
#include "graphics/ElementBufferedRenderer.h"
#include "realm/Realm.h"
#include "ui/MainWindow.h"
#include "util/Timer.h"
#include "util/Util.h"

#include <array>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

	void ElementBufferedRenderer::init() {
		if (!realm)
			return;
		if (initialized)
			reset();
		assert(realm);
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

	void ElementBufferedRenderer::setup(TileProvider &provider_) {
		// layer = layer_;
		provider = &provider_;
	}

	void ElementBufferedRenderer::render(float /* divisor */, float scale, float center_x, float center_y) {
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

		const auto [chunk_x, chunk_y] = chunkPosition.copyBase();
		auto &tileset = realm->getTileset();
		const auto tilesize = tileset.getTileSize();
		auto texture = tileset.getTexture(realm->getGame());

		glm::mat4 projection(1.f);
		projection = glm::scale(projection, {float(tilesize), -float(tilesize), 1.f}) *
		             glm::scale(projection, {scale / backbufferWidth, scale / backbufferHeight, 1.f}) *
		             glm::translate(projection, {
		                 center_x - CHUNK_SIZE / 2.f + chunk_x * CHUNK_SIZE,
		                 center_y - CHUNK_SIZE / 2.f + chunk_y * CHUNK_SIZE,
		                 0.f
		             });

		CHECKGL

		shader.bind();
		vao.bind();
		vbo.bind();
		ebo.bind();
		// Try commenting this out, it's kinda funny
		texture->bind(0);
		shader.set("texture0", 0);
		shader.set("projection", projection);
		// shader.set("divisor", divisor);
		// shader.set("bright_tiles", brightTiles);

		GL::triangles(CHUNK_SIZE * CHUNK_SIZE);
	}

	void ElementBufferedRenderer::render(float /* divisor */) {
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
		// shader.set("divisor", divisor);
		// shader.set("bright_tiles", brightTiles);

		GL::triangles(CHUNK_SIZE * CHUNK_SIZE);
	}

	bool ElementBufferedRenderer::reupload() {
		return generateVertexBufferObject() && generateVertexArrayObject();
	}

	std::future<bool> ElementBufferedRenderer::queueReupload() {
		assert(realm != nullptr);

		auto promise = std::make_shared<std::promise<bool>>();
		std::future<bool> future = promise->get_future();
		ClientGamePtr client_game = realm->getGame().toClientPointer();

		client_game->getWindow().queue([this, promise, client_game]() {
			client_game->activateContext();
			promise->set_value(reupload());
		});

		return future;
	}

	bool ElementBufferedRenderer::onBackbufferResized(int width, int height) {
		if (width == backbufferWidth && height == backbufferHeight)
			return false;
		backbufferWidth = width;
		backbufferHeight = height;
		return true;
	}

	void ElementBufferedRenderer::setChunk(TileChunk &new_chunk, bool can_reupload) {
		if (&new_chunk == chunk)
			return;
		chunk = &new_chunk;
		if (can_reupload) {
			Timer timer{"EBR::setChunk::reupload"};
			reupload();
		}
	}

	void ElementBufferedRenderer::setChunkPosition(const ChunkPosition &new_pos) {
		auto lock = chunkPosition.uniqueLock();
		if (new_pos != chunkPosition) {
			chunkPosition.getBase() = new_pos;
			positionDirty = true;
		}
	}

	void ElementBufferedRenderer::snooze() {
		reset();
	}

	void ElementBufferedRenderer::wakeUp() {
		init();
	}

	bool ElementBufferedRenderer::generateVertexBufferObject() {
		assert(realm);

		auto &tileset = realm->getTileset();
		const auto tilesize = tileset.getTileSize();
		const auto tileset_width = tileset.getTexture(realm->getGame())->width;

		const auto set_width = tileset_width / tilesize;

		if (set_width == 0)
			return false;

		const float divisor = set_width;
		const float t_size = 1.f / divisor - TILE_TEXTURE_PADDING * 2;

		isMissing = false;

		const TileID missing = tileset["base:tile/void"];
		Game &game = realm->getGame();

		Timer timer{"BufferedVBOInit"};
		vbo.init<float, 19>(CHUNK_SIZE, CHUNK_SIZE, GL_STATIC_DRAW, [this, &game, &tileset, set_width, divisor, t_size, missing](size_t x, size_t y) {
			const auto [chunk_x, chunk_y] = chunkPosition.copyBase();

			std::array<TileID, LAYER_COUNT> tiles{}, uppers{};

			for (uint8_t layer_index = 1; layer_index <= LAYER_COUNT; ++layer_index) {
				Layer layer = getLayer(layer_index);

				const std::optional<TileID> tile_opt = realm->tryTile(layer, Position{
					static_cast<Index>(y) + CHUNK_SIZE * (chunk_y + 1), // why `+ 1`?
					static_cast<Index>(x) + CHUNK_SIZE * (chunk_x + 1)  // here too
				});

				if (!tile_opt) {
					isMissing = true;
					tiles[layer_index - 1] = missing;
				} else
					tiles[layer_index - 1] = *tile_opt;

				const std::optional<TileID> upper_opt = realm->tryTile(layer, Position{
					static_cast<Index>(y + 1) + CHUNK_SIZE * (chunk_y + 1),
					static_cast<Index>(x)     + CHUNK_SIZE * (chunk_x + 1)
				});

				if (upper_opt)
					uppers[layer_index - 1] = tileset.getUpper(*upper_opt);
				else
					uppers[layer_index - 1] = 0;
			}

			const auto fluid_opt = realm->tryFluid({
				Index(y) + CHUNK_SIZE * (chunk_y + 1),
				Index(x) + CHUNK_SIZE * (chunk_x + 1)
			});

			TileID fluid_tile = -1;
			float fluid_opacity;

			if (fluid_opt) {
				if (auto tile_opt = game.getFluidTileID(fluid_opt->id)) {
					fluid_tile = *tile_opt;
					if (FluidTile::FULL <= fluid_opt->level)
						fluid_opacity = 1.f;
					else
						fluid_opacity = float(fluid_opt->level) / FluidTile::FULL;
				}
			}

			if (fluid_tile == static_cast<TileID>(-1)) {
				isMissing = true;
				fluid_tile = missing;
				fluid_opacity = 0.f;
			}

			static_assert(LAYER_COUNT == 4);

			// Texture coordinates for the base tile
#define T_DEFS(I) \
			const float tx##I = (tiles[I] % set_width) / divisor + TILE_TEXTURE_PADDING; \
			const float ty##I = (tiles[I] / set_width) / divisor + TILE_TEXTURE_PADDING;

			// Texture coordinates for the upper portion of the below tile
#define U_DEFS(I) \
			const float ux##I = (uppers[I] % set_width) / divisor + TILE_TEXTURE_PADDING; \
			const float uy##I = (uppers[I] / set_width) / divisor + TILE_TEXTURE_PADDING;

			T_DEFS(0); T_DEFS(1); T_DEFS(2); T_DEFS(3);
			U_DEFS(0); U_DEFS(1); U_DEFS(2); U_DEFS(3);

			const float fx0 = (fluid_tile % set_width) / divisor + TILE_TEXTURE_PADDING;
			const float fy0 = (fluid_tile / set_width) / divisor + TILE_TEXTURE_PADDING;

#define T_ARR_0(I) tx##I, ty##I
#define T_ARR_1(I) tx##I + t_size, ty##I
#define T_ARR_2(I) tx##I, ty##I + t_size
#define T_ARR_3(I) tx##I + t_size, ty##I + t_size
#define T_ARR_N(N) T_ARR_##N(0), T_ARR_##N(1), T_ARR_##N(2), T_ARR_##N(3)

#define U_ARR_0(I) ux##I, uy##I
#define U_ARR_1(I) ux##I + t_size, uy##I
#define U_ARR_2(I) ux##I, uy##I + t_size
#define U_ARR_3(I) ux##I + t_size, uy##I + t_size
#define U_ARR_N(N) U_ARR_##N(0), U_ARR_##N(1), U_ARR_##N(2), U_ARR_##N(3)

#define F_ARR_0 fx0, fy0
#define F_ARR_1 fx0 + t_size, fy0
#define F_ARR_2 fx0, fy0 + t_size
#define F_ARR_3 fx0 + t_size, fy0 + t_size

			return std::array{
				std::array{T_ARR_N(0), U_ARR_N(0), F_ARR_0, fluid_opacity},
				std::array{T_ARR_N(1), U_ARR_N(1), F_ARR_1, fluid_opacity},
				std::array{T_ARR_N(2), U_ARR_N(2), F_ARR_2, fluid_opacity},
				std::array{T_ARR_N(3), U_ARR_N(3), F_ARR_3, fluid_opacity},
			};
		});

		return vbo.getHandle() != 0;
	}

	bool ElementBufferedRenderer::generateElementBufferObject() {
		uint32_t i = 0;
		ebo.init<uint32_t, 6>(CHUNK_SIZE, CHUNK_SIZE, GL_STATIC_DRAW, [&i](size_t, size_t) {
			i += 4;
			return std::array{i - 4, i - 3, i - 2, i - 3, i - 2, i - 1};
		});
		return ebo.getHandle() != 0;
	}

	bool ElementBufferedRenderer::generateVertexArrayObject() {
		if (vbo.getHandle() != 0)
			vao.init(vbo, {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1});
		return vao.getHandle() != 0;
	}

	bool ElementBufferedRenderer::generateLightingTexture() {
		return true;
	}

	void ElementBufferedRenderer::recomputeLighting() {
		return;

		/*
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
			reshader.set("xs", float(width));
			reshader.set("ys", float(height));
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
		//*/
	}

	void ElementBufferedRenderer::check(int handle, bool is_link) {
		int success;
		std::array<char, 2048> info{};
		if (is_link)
			glGetProgramiv(handle, GL_LINK_STATUS, &success);
		else
			glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLsizei len = 666;
			if (is_link)
				glGetProgramInfoLog(handle, GL_INFO_LOG_LENGTH, &len, info.data());
			else
				glGetShaderInfoLog(handle, info.size(), &len, info.data());
			std::cerr << "Error with " << handle << " (l=" << len << "): " << info.data() << '\n';
		}
	}
}
