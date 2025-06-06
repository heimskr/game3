// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.ull_source/sprite_renderer.cpp

#include "graphics/Shader.h"

#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "graphics/BatchSpriteRenderer.h"
#include "graphics/GL.h"
#include "graphics/Util.h"
#include "ui/Window.h"
#include "util/FS.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		const std::string & spriteFrag() { static auto out = readFile("resources/sprite_batch.frag"); return out; }
		const std::string & spriteVert() { static auto out = readFile("resources/sprite_batch.vert"); return out; }
	}

	BatchSpriteRenderer::BatchSpriteRenderer(Window &window):
		SpriteRenderer(window),
		shader("BatchSpriteRenderer") {
			shader.init(spriteVert(), spriteFrag());
			initRenderData();
		}

	BatchSpriteRenderer::~BatchSpriteRenderer() {
		remove();
	}

	void BatchSpriteRenderer::remove() {
		initialized = false;
	}

	void BatchSpriteRenderer::update(const Window &window) {
		const int backbuffer_width  = window.getWidth();
		const int backbuffer_height = window.getHeight();
		const double scale = window.scale;

		if (backbuffer_width != backbufferWidth || backbuffer_height != backbufferHeight) {
			HasBackbuffer::update(backbuffer_width, backbuffer_height);
			shader.bind();
			shader.set("screenSize", Vector2d(backbuffer_width, backbuffer_height));
		}

		if (scale != canvasScale) {
			canvasScale = scale;
			shader.bind();
			shader.set("canvasScale", float(scale));
		}

		if (window.center.first != centerX || window.center.second != centerY) {
			centerX = window.center.first;
			centerY = window.center.second;
			shader.bind();
			shader.set("center", Vector2d(float(centerX), float(centerY)));
		}
	}

	void BatchSpriteRenderer::update(int width, int height) {
		if (width != backbufferWidth || height != backbufferHeight) {
			backbufferWidth = width;
			backbufferHeight = height;
			shader.bind();
			shader.set("screenSize", Vector2d(width, height));
		}
	}

	void BatchSpriteRenderer::drawOnMap(const std::shared_ptr<Texture> &texture, double x, double y, double scale, double angle, double alpha) {
		drawOnMap(texture, RenderOptions{
			.x = x,
			.y = y,
			.sizeX = double(texture->width),
			.sizeY = double(texture->height),
			.scaleX = scale,
			.scaleY = scale,
			.angle = angle,
			.color{1.f, 1.f, 1.f, static_cast<float>(alpha)}
		});
	}

	void BatchSpriteRenderer::drawOnMap(const std::shared_ptr<Texture> &texture, const RenderOptions &options) {
		if (!initialized)
			return;

		RenderOptions modified = options;

		if (modified.sizeX < 0)
			modified.sizeX = texture->width;
		if (modified.sizeY < 0)
			modified.sizeY = texture->height;

		batchItems.emplace_back(texture, modified);
	}

	void BatchSpriteRenderer::renderNow() {
		if (!initialized || batchItems.empty())
			return;

		std::shared_ptr<Texture> last_texture = batchItems.front().texture;

		std::vector<const RenderOptions *> buffer;

		constexpr static size_t BUFFER_CAPACITY = 1024;

		const size_t tile_size = window->game->getActiveRealm()->getTileset().getTileSize();

		for (const auto &[texture, options]: batchItems) {
			if (texture != last_texture || buffer.size() >= BUFFER_CAPACITY) {
				flush(last_texture, buffer, tile_size);
				buffer.clear();
				last_texture = texture;
			}

			buffer.push_back(&options);
		}

		if (!buffer.empty())
			flush(last_texture, buffer, tile_size);

		batchItems.clear();
	}

	void BatchSpriteRenderer::reset() {
		shader.init(spriteVert(), spriteFrag());
		initRenderData();
	}

	void BatchSpriteRenderer::initRenderData() {
		shader.bind();
		shader.set("mapLength", float(CHUNK_SIZE * REALM_DIAMETER));
		initialized = true;
	}

	void BatchSpriteRenderer::flush(std::shared_ptr<Texture> texture, const std::vector<const RenderOptions *> &options, size_t tile_size) {
		assert(texture != nullptr);
		Atlas *atlas_ptr = nullptr;

		if (auto iter = atlases.find(texture->id); iter != atlases.end()) {
			atlas_ptr = &iter->second;
			std::vector<float> data = generateData(atlas_ptr->texture, options);
			if (atlas_ptr->lastDataCount < data.size()) {
				atlas_ptr->lastDataCount = data.size();
				atlas_ptr->vbo.update(data, false);
			} else {
				atlas_ptr->vbo.update(data, true);
			}
		} else {
			atlas_ptr = &(atlases[texture->id] = generateAtlas(texture, options));
		}

		if (!atlas_ptr)
			throw std::runtime_error("Couldn't find or initialize Atlas in BatchSpriteRenderer::flush");

		Atlas &atlas = *atlas_ptr;
		shader.bind();
		shader.set("atlasSize", Vector2d(atlas.texture->width, atlas.texture->height));
		shader.set("tileSize", float(tile_size));
		shader.set("sprite", 0);
		atlas.vao.bind();
		atlas.vbo.bind();
		texture->bind(0);
		glEnable(GL_BLEND); CHECKGL
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6 * options.size()); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	auto BatchSpriteRenderer::generateAtlas(std::shared_ptr<Texture> texture, const std::vector<const RenderOptions *> &options) -> Atlas {
		Atlas atlas;
		atlas.texture = texture;
		std::vector<float> data = generateData(texture, options);
		atlas.vbo.init(data.data(), data.size(), GL_DYNAMIC_DRAW);
		atlas.lastDataCount = data.size();
		atlas.vao.init(atlas.vbo, {2, 2, 2, 2, 1, 1, 4, 2, 4, 4});
		return atlas;
	}

	std::vector<float> BatchSpriteRenderer::generateData(std::shared_ptr<Texture> texture, const std::vector<const RenderOptions *> &options) {
		std::vector<float> data;
		data.reserve(options.size() * 20);

		const int texture_width  = texture->width;
		const int texture_height = texture->height;

		for (const RenderOptions *item: options) {
			for (const auto &[x, y]: std::initializer_list<std::pair<float, float>>{{0.f, 1.f}, {1.f, 0.f}, {0.f, 0.f}, {0.f, 1.f}, {1.f, 1.f}, {1.f, 0.f}}) {
				data.push_back(x);
				data.push_back(y);
				data.push_back(item->x);
				data.push_back(item->y);
				data.push_back(item->offsetX);
				data.push_back(item->offsetY);
				data.push_back(item->scaleX);
				data.push_back(item->scaleY);
				data.push_back(item->invertY? -1.f : 1.f);
				data.push_back(item->angle);
				data.push_back(item->color.red);
				data.push_back(item->color.green);
				data.push_back(item->color.blue);
				data.push_back(item->color.alpha);
				data.push_back(item->sizeX);
				data.push_back(item->sizeY);
				data.push_back(item->offsetX * 2. / texture_width);
				data.push_back(item->offsetY * 2. / texture_height);
				data.push_back(item->sizeX / texture_width);
				data.push_back(item->sizeY / texture_height);
				data.push_back(item->composite.red);
				data.push_back(item->composite.green);
				data.push_back(item->composite.blue);
				data.push_back(item->composite.alpha);
			}
		}

		return data;
	}
}
