// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.ull_source/sprite_renderer.cpp

#include <iostream>

#include "graphics/Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "graphics/GL.h"
#include "graphics/SingleSpriteRenderer.h"
#include "ui/Canvas.h"
#include "util/FS.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		const std::string & spriteFrag() { static auto out = readFile("resources/sprite.frag"); return out; }
		const std::string & spriteVert() { static auto out = readFile("resources/sprite.vert"); return out; }
	}

	SingleSpriteRenderer::SingleSpriteRenderer(Canvas &canvas_): SpriteRenderer(canvas_), shader("SingleSpriteRenderer") {
		shader.init(spriteVert(), spriteFrag());
		initRenderData();
	}

	SingleSpriteRenderer::SingleSpriteRenderer(SingleSpriteRenderer &&other): SingleSpriteRenderer(*other.canvas) {
		other.canvas = nullptr;
		shader = std::move(other.shader);
		quadVAO = other.quadVAO;
		initialized = other.initialized;
		backbufferWidth = other.backbufferWidth;
		backbufferHeight = other.backbufferHeight;
		other.quadVAO = 0;
		other.initialized = false;
		other.backbufferWidth = -1;
		other.backbufferHeight = -1;
	}

	SingleSpriteRenderer::~SingleSpriteRenderer() {
		remove();
	}

	void SingleSpriteRenderer::remove() {
		if (initialized) {
			glDeleteVertexArrays(1, &quadVAO);
			quadVAO = 0;
			initialized = false;
		}
	}

	SingleSpriteRenderer & SingleSpriteRenderer::operator=(SingleSpriteRenderer &&other) {
		canvas = other.canvas;
		other.canvas = nullptr;
		shader.reset();
		shader = std::move(other.shader);
		quadVAO = other.quadVAO;
		initialized = other.initialized;
		backbufferWidth = other.backbufferWidth;
		backbufferHeight = other.backbufferHeight;
		other.quadVAO = 0;
		other.initialized = false;
		other.backbufferWidth = -1;
		other.backbufferHeight = -1;
		return *this;
	}

	void SingleSpriteRenderer::update(const Canvas &) {
		const int backbuffer_width  = canvas->width();
		const int backbuffer_height = canvas->height();

		if (backbuffer_width != backbufferWidth || backbuffer_height != backbufferHeight) {
			backbufferWidth = backbuffer_width;
			backbufferHeight = backbuffer_height;
			glm::mat4 projection = glm::ortho(0., double(backbuffer_width), double(backbuffer_height), 0., -1., 1.);
			shader.bind();
			shader.set("projection", projection);
		}
	}

	void SingleSpriteRenderer::drawOnMap(const std::shared_ptr<Texture> &texture, double x, double y, double scale, double angle, double alpha) {
		drawOnMap(texture, RenderOptions {
			.x = x,
			.y = y,
			.sizeX = double(texture->width),
			.sizeY = double(texture->height),
			.scaleX = scale,
			.scaleY = scale,
			.angle = angle,
			.color = {1.f, 1.f, 1.f, float(alpha)},
		});
	}

	void SingleSpriteRenderer::drawOnMap(const std::shared_ptr<Texture> &texture, RenderOptions options) {
		if (!initialized)
			return;

		if (options.sizeX < 0)
			options.sizeX = texture->width;
		if (options.sizeY < 0)
			options.sizeY = texture->height;

		RealmPtr realm = canvas->game->activeRealm.copyBase();
		TileProvider &provider = realm->tileProvider;
		TilesetPtr tileset     = provider.getTileset(*canvas->game);
		const auto tile_size   = tileset->getTileSize();
		const auto map_length  = CHUNK_SIZE * REALM_DIAMETER;

		options.x *= tile_size * canvas->scale / 2.;
		options.y *= tile_size * canvas->scale / 2.;

		options.x += canvas->width() / 2.;
		options.x -= map_length * tile_size * canvas->scale / canvas->magic * 2.; // TODO: the math here is a little sus... things might cancel out
		options.x += centerX * canvas->scale * tile_size / 2.;

		options.y += canvas->height() / 2.;
		options.y -= map_length * tile_size * canvas->scale / canvas->magic * 2.;
		options.y += centerY * canvas->scale * tile_size / 2.;

		shader.bind();

		glm::mat4 model = glm::mat4(1.);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(options.x - options.offsetX * canvas->scale * options.scaleX, options.y - options.offsetY * canvas->scale * options.scaleY, 0.));
		model = glm::translate(model, glm::vec3(.5 * texture->width, .5 * texture->height, 0.)); // move origin of rotation to center of quad
		model = glm::rotate(model, float(glm::radians(options.angle)), glm::vec3(0., 0., 1.)); // then rotate
		model = glm::translate(model, glm::vec3(-.5 * texture->width, -.5 * texture->height, 0.)); // move origin back
		model = glm::scale(model, glm::vec3(texture->width * options.scaleX * canvas->scale / 2., texture->height * options.scaleY * canvas->scale / 2., 2.)); // last scale

		shader.set("model", model);
		shader.set("spriteColor", options.color.red, options.color.green, options.color.blue, options.color.alpha);
		const double multiplier = 2. / texture->width;
		shader.set("texturePosition", options.offsetX * multiplier, options.offsetY * multiplier, options.sizeX / texture->width, options.sizeY / texture->width);

		glActiveTexture(GL_TEXTURE0);
		texture->bind();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	void SingleSpriteRenderer::reset() {
		shader.init(spriteVert(), spriteFrag());
		initRenderData();
	}

	void SingleSpriteRenderer::initRenderData() {
		if (initialized)
			glDeleteVertexArrays(1, &quadVAO);

		unsigned int vbo;
		static const float vertices[] {
			// pos    // tex
			0., 1., 0., 1.,
			1., 0., 1., 0.,
			0., 0., 0., 0.,

			0., 1., 0., 1.,
			1., 1., 1., 1.,
			1., 0., 1., 0.
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindVertexArray(quadVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		initialized = true;
	}

	void SingleSpriteRenderer::setupShader(int texture_width, int texture_height, const RenderOptions &options) {
		glm::mat4 model = glm::mat4(1.);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(options.x * 16.f - options.offsetX * 2. * options.scaleX, options.y * 16.f - options.offsetY * 2. * options.scaleY, 0.0f));
		if (options.invertY)
			model = glm::scale(model, glm::vec3(1., -1., 1.));
		model = glm::translate(model, glm::vec3(.5 * texture_width, .5 * texture_height, 0.0f));
		model = glm::rotate   (model, float(glm::radians(options.angle)), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-.5 * texture_width, -.5 * texture_height, 0.0f));
		model = glm::scale    (model, glm::vec3(texture_width * options.scaleX, texture_height * options.scaleY, 1.0f));

		shader.bind();
		shader.set("model", model);
		shader.set("spriteColor", options.color.red, options.color.green, options.color.blue, options.color.alpha);
		const double multiplier = 2.;
		const double multiplier_x = multiplier / texture_width;
		const double multiplier_y = multiplier / texture_height;
		shader.set("texturePosition", options.offsetX * multiplier_x, options.offsetY * multiplier_y, options.sizeX / texture_width, options.sizeY / texture_height);
	}
}
