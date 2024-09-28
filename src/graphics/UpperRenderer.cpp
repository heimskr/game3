// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Renderers/UpperRenderer.cs

#include "game/ClientGame.h"
#include "game/Game.h"
#include "graphics/Shader.h"
#include "graphics/Tileset.h"
#include "graphics/UpperRenderer.h"
#include "realm/Realm.h"
#include "ui/Window.h"
#include "util/FS.h"
#include "util/Timer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <cassert>

namespace Game3 {
	namespace {
		const std::string & upperFrag() { static auto out = readFile("resources/upper.frag"); return out; }
		const std::string & upperVert() { static auto out = readFile("resources/upper.vert"); return out; }
		constexpr float TILE_TEXTURE_PADDING = 1.f / 16384.f;
	}

	UpperRenderer::UpperRenderer() = default;

	UpperRenderer::UpperRenderer(Realm &realm):
		realm(&realm) {}

	UpperRenderer::~UpperRenderer() {
		reset();
	}

	void UpperRenderer::reset() {
		if (initialized) {
			vao.reset();
			ebo.reset();
			vbo.reset();
			shader.reset();
			tileset.reset();
			fbo.reset();
			initialized = false;
		}
	}

	void UpperRenderer::init() {
		if (!realm)
			return;
		if (initialized)
			reset();
		assert(realm);
		shader.init(upperVert(), upperFrag());
		generateVertexBufferObject();
		generateElementBufferObject();
		generateVertexArrayObject();
		fbo.init();
		initialized = true;
	}

	void UpperRenderer::setup(TileProvider &provider_) {
		provider = &provider_;
	}

	void UpperRenderer::render(float /* divisor */, float scale, float center_x, float center_y) {
		if (!initialized)
			return;

		assert(realm);

		if (vbo.getHandle() == 0 || positionDirty) {
			if (!reupload())
				return;
			positionDirty = false;
		}

		const auto [chunk_x, chunk_y] = chunkPosition.copyBase();
		auto &tileset = realm->getTileset();
		const auto tilesize = tileset.getTileSize();
		auto texture = tileset.getTexture(*realm->getGame());

		glm::mat4 projection(1.f);
		projection = glm::scale(projection, {float(tilesize), -float(tilesize), 1.f}) *
		             glm::scale(projection, {scale / float(backbufferWidth), scale / float(backbufferHeight), 1.f}) *
		             glm::translate(projection, {
		                 center_x - CHUNK_SIZE / 2.f + float(chunk_x) * CHUNK_SIZE,
		                 center_y - CHUNK_SIZE / 2.f + float(chunk_y) * CHUNK_SIZE,
		                 0.f
		             });

		CHECKGL

		shader.bind();
		vao.bind();
		vbo.bind();
		ebo.bind();
		texture->bind(0);
		shader.set("texture0", 0);
		shader.set("projection", projection);

		glEnable(GL_BLEND); CHECKGL
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECKGL
		GL::triangles(CHUNK_SIZE * CHUNK_SIZE);
	}

	void UpperRenderer::render(float /* divisor */) {
		if (!initialized)
			return;

		assert(realm);

		if (vbo.getHandle() == 0 || positionDirty) {
			if (!reupload())
				return;
			positionDirty = false;
		}

		auto &tileset = realm->getTileset();
		const auto tilesize = tileset.getTileSize();

		glm::mat4 projection(1.f);
		projection = glm::scale(projection, {tilesize, tilesize, 1.f}) *
		             glm::scale(projection, {2.f / float(backbufferWidth), 2.f / float(backbufferHeight), 1.f}) *
		             glm::translate(projection, {-CHUNK_SIZE, -CHUNK_SIZE, 0.f});

		shader.bind();
		vao.bind();
		vbo.bind();
		ebo.bind();
		tileset.getTexture(*realm->getGame())->bind(0);
		shader.set("texture0", 0);
		shader.set("projection", projection);

		glEnable(GL_BLEND); CHECKGL
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECKGL
		GL::triangles(CHUNK_SIZE * CHUNK_SIZE);
	}

	bool UpperRenderer::reupload() {
		return generateVertexBufferObject() && generateVertexArrayObject();
	}

	std::future<bool> UpperRenderer::queueReupload() {
		assert(realm != nullptr);

		auto promise = std::make_shared<std::promise<bool>>();
		std::future<bool> future = promise->get_future();
		ClientGamePtr client_game = realm->getGame()->toClientPointer();

		client_game->getWindow()->queue([this, promise, client_game](Window &) {
			client_game->activateContext();
			promise->set_value(reupload());
		});

		return future;
	}

	bool UpperRenderer::onBackbufferResized(int width, int height) {
		if (width == backbufferWidth && height == backbufferHeight)
			return false;
		backbufferWidth = width;
		backbufferHeight = height;
		return true;
	}

	void UpperRenderer::setChunk(TileChunk &new_chunk, bool can_reupload) {
		if (&new_chunk == chunk)
			return;
		chunk = &new_chunk;
		if (can_reupload) {
			Timer timer{"UR::setChunk::reupload"};
			reupload();
		}
	}

	void UpperRenderer::setChunkPosition(const ChunkPosition &new_pos) {
		auto lock = chunkPosition.uniqueLock();
		if (new_pos != chunkPosition) {
			chunkPosition.getBase() = new_pos;
			positionDirty = true;
		}
	}

	void UpperRenderer::snooze() {
		reset();
	}

	void UpperRenderer::wakeUp() {
		init();
	}

	bool UpperRenderer::generateVertexBufferObject() {
		assert(realm);

		Tileset &tileset = realm->getTileset();
		const auto tilesize = tileset.getTileSize();
		const auto tileset_width = tileset.getTexture(*realm->getGame())->width;

		const auto set_width = tileset_width / tilesize;

		if (set_width == 0)
			return false;

		const float divisor(set_width);
		const float t_size = 1.f / divisor - TILE_TEXTURE_PADDING * 2;

		Timer timer{"UpperVBOInit"};
		vbo.init<float, 8>(CHUNK_SIZE, CHUNK_SIZE, GL_STATIC_DRAW, [this, &tileset, set_width, divisor, t_size](size_t x, size_t y) {
			const auto [chunk_x, chunk_y] = chunkPosition.copyBase();

			std::array<TileID, LAYER_COUNT> uppers{};

			for (uint8_t layer_index = 1; layer_index <= LAYER_COUNT; ++layer_index) {
				const std::optional<TileID> upper_opt = realm->tryTile(getLayer(layer_index), Position{
					static_cast<Index>(y + 1) + CHUNK_SIZE * (chunk_y + 1),
					static_cast<Index>(x)     + CHUNK_SIZE * (chunk_x + 1)
				});

				if (upper_opt)
					uppers[layer_index - 1] = tileset.getUpper(*upper_opt);
				else
					uppers[layer_index - 1] = 0;
			}

			static_assert(LAYER_COUNT == 4);

			// Texture coordinates for the upper portion of the below tile
#define U_DEFS(I) \
			const float ux##I((uppers[I] % set_width) / divisor + TILE_TEXTURE_PADDING); \
			const float uy##I((uppers[I] / set_width) / divisor + TILE_TEXTURE_PADDING);

			U_DEFS(0); U_DEFS(1); U_DEFS(2); U_DEFS(3);

#define U_ARR_0(I) ux##I, uy##I
#define U_ARR_1(I) ux##I + t_size, uy##I
#define U_ARR_2(I) ux##I, uy##I + t_size
#define U_ARR_3(I) ux##I + t_size, uy##I + t_size
#define U_ARR_N(N) U_ARR_##N(0), U_ARR_##N(1), U_ARR_##N(2), U_ARR_##N(3)

			return std::array{
				std::array{U_ARR_N(0)},
				std::array{U_ARR_N(1)},
				std::array{U_ARR_N(2)},
				std::array{U_ARR_N(3)},
			};
		});

		return vbo.getHandle() != 0;
	}

	bool UpperRenderer::generateElementBufferObject() {
		uint32_t i = 0;
		ebo.init<uint32_t, 6>(CHUNK_SIZE, CHUNK_SIZE, GL_STATIC_DRAW, [&i](size_t, size_t) {
			i += 4;
			return std::array{i - 4, i - 3, i - 2, i - 3, i - 2, i - 1};
		});
		return ebo.getHandle() != 0;
	}

	bool UpperRenderer::generateVertexArrayObject() {
		if (vbo.getHandle() != 0)
			vao.init(vbo, {2, 2, 2, 2, 2});
		return vao.getHandle() != 0;
	}

	void UpperRenderer::check(int handle, bool is_link) {
		int success{};
		std::array<char, 2048> info{"No info available"};

		if (is_link) {
			glGetProgramiv(handle, GL_LINK_STATUS, &success); CHECKGL
		} else {
			glGetShaderiv(handle, GL_COMPILE_STATUS, &success); CHECKGL
		}

		if (!success) {
			GLsizei len = 666;

			if (is_link) {
				glGetProgramInfoLog(handle, GL_INFO_LOG_LENGTH, &len, info.data()); CHECKGL
			} else {
				glGetShaderInfoLog(handle, info.size(), &len, info.data()); CHECKGL
			}

			INFO("UpperRenderer.cpp: error with handle {} (len={}): {}", handle, len, info.data());
		}
	}
}
