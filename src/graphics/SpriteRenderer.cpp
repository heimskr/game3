// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.ull_source/sprite_renderer.cpp

#include <iostream>

#include "graphics/Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "graphics/Texture.h"
#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "graphics/GL.h"
#include "graphics/SpriteRenderer.h"
#include "ui/Canvas.h"
#include "util/FS.h"
#include "util/Util.h"

namespace Game3 {
	namespace {
		std::string sprite_frag = readFile("resources/sprite.frag");
		std::string sprite_vert = readFile("resources/sprite.vert");
	}

	SpriteRenderer::SpriteRenderer(Canvas &canvas_): canvas(&canvas_), shader("SpriteRenderer") {
		shader.init(sprite_vert, sprite_frag);
		initRenderData();
	}

	SpriteRenderer::SpriteRenderer(SpriteRenderer &&other): SpriteRenderer(*other.canvas) {
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

	SpriteRenderer::~SpriteRenderer() {
		remove();
	}

	void SpriteRenderer::remove() {
		if (initialized) {
			glDeleteVertexArrays(1, &quadVAO);
			quadVAO = 0;
			initialized = false;
		}
	}

	SpriteRenderer & SpriteRenderer::operator=(SpriteRenderer &&other) {
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

	void SpriteRenderer::update(int backbuffer_width, int backbuffer_height) {
		if (backbuffer_width != backbufferWidth || backbuffer_height != backbufferHeight) {
			backbufferWidth = backbuffer_width;
			backbufferHeight = backbuffer_height;
			glm::mat4 projection = glm::ortho(0., double(backbuffer_width), double(backbuffer_height), 0., -1., 1.);
			shader.bind();
			shader.set("projection", projection);
		}
	}

	void SpriteRenderer::drawOnMap(Texture &texture, double x, double y, double scale, double angle, double alpha) {
		drawOnMap(texture, RenderOptions {
			.x = x,
			.y = y,
			.sizeX = double(texture.width),
			.sizeY = double(texture.height),
			.scaleX = scale,
			.scaleY = scale,
			.angle = angle,
			.alpha = alpha
		});
	}

	void SpriteRenderer::drawOnMap(Texture &texture, RenderOptions options) {
		if (!initialized)
			return;

		if (options.sizeX < 0)
			options.sizeX = texture.width;
		if (options.sizeY < 0)
			options.sizeY = texture.height;

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
		model = glm::translate(model, glm::vec3(options.x - options.xOffset * canvas->scale * options.scaleX, options.y - options.yOffset * canvas->scale * options.scaleY, 0.));
		model = glm::translate(model, glm::vec3(.5 * texture.width, .5 * texture.height, 0.)); // move origin of rotation to center of quad
		model = glm::rotate(model, float(glm::radians(options.angle)), glm::vec3(0., 0., 1.)); // then rotate
		model = glm::translate(model, glm::vec3(-.5 * texture.width, -.5 * texture.height, 0.)); // move origin back
		model = glm::scale(model, glm::vec3(texture.width * options.scaleX * canvas->scale / 2., texture.height * options.scaleY * canvas->scale / 2., 2.)); // last scale

		shader.set("model", model);
		shader.set("spriteColor", 1., 1., 1., options.alpha);
		const double multiplier = 2. / texture.width;
		shader.set("texturePosition", options.xOffset * multiplier, options.yOffset * multiplier, options.sizeX / texture.width, options.sizeY / texture.width);
		// shader.set("divisor", divisor);

		glActiveTexture(GL_TEXTURE0);
		texture.bind();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	void SpriteRenderer::drawOnMap(GL::Texture &texture, RenderOptions options) {
		if (!initialized)
			return;

		const auto texture_width = texture.getWidth();
		const auto texture_height = texture.getHeight();

		if (options.sizeX < 0)
			options.sizeX = texture_width;
		if (options.sizeY < 0)
			options.sizeY = texture_height;

		TileProvider &provider = canvas->game->activeRealm->tileProvider;
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

		glm::mat4 model(1.);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(options.x - options.xOffset * canvas->scale * options.scaleX, options.y - options.yOffset * canvas->scale * options.scaleY, 0.));
		model = glm::translate(model, glm::vec3(.5 * texture_width, .5 * texture_height, 0.)); // move origin of rotation to center of quad
		model = glm::rotate(model, float(glm::radians(options.angle)), glm::vec3(0., 0., 1.)); // then rotate
		model = glm::translate(model, glm::vec3(-.5 * texture_width, -.5 * texture_height, 0.)); // move origin back
		model = glm::scale(model, glm::vec3(texture_width * options.scaleX * canvas->scale / 2., texture_height * options.scaleY * canvas->scale / 2., 2.)); // last scale

		shader.set("model", model);
		shader.set("spriteColor", 1., 1., 1., options.alpha);
		const double multiplier = 2. / texture_width;
		shader.set("texturePosition", options.xOffset * multiplier, options.yOffset * multiplier, options.sizeX / texture_width, options.sizeY / texture_width);
		// shader.set("divisor", divisor);

		texture.bind(0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	void SpriteRenderer::drawOnScreen(Texture &texture, double x, double y, double scale, double angle, double alpha) {
		drawOnScreen(texture, RenderOptions {
			.x = x,
			.y = y,
			.sizeX = static_cast<double>(texture.width),
			.sizeY = static_cast<double>(texture.height),
			.scaleX = scale,
			.scaleY = scale,
			.angle = angle,
			.alpha = alpha
		});
	}

	void SpriteRenderer::drawOnScreen(Texture &texture, RenderOptions options) {
		if (!initialized)
			return;

		const auto texture_width  = texture.width;
		const auto texture_height = texture.height;
		if (options.hackY)
			hackY(options.y, options.yOffset, options.scaleY);

		options.sizeX = options.sizeX < 0.? -options.sizeX * texture_width  : options.sizeX;
		options.sizeY = options.sizeY < 0.? -options.sizeY * texture_height : options.sizeY;
		setupShader(texture_width, texture_height, options);

		glActiveTexture(GL_TEXTURE0);
		texture.bind();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	void SpriteRenderer::drawOnScreen(GL::Texture &texture, RenderOptions options) {
		if (!initialized)
			return;

		const auto texture_width  = texture.getWidth();
		const auto texture_height = texture.getHeight();
		if (options.hackY)
			hackY(options.y, options.yOffset, options.scaleY);

		options.sizeX = options.sizeX < 0.? -options.sizeX * texture_width  : options.sizeX;
		options.sizeY = options.sizeY < 0.? -options.sizeY * texture_height : options.sizeY;
		setupShader(texture_width, texture_height, options);

		texture.bind(0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	void SpriteRenderer::reset() {
		shader.init(sprite_vert, sprite_frag);
		initRenderData();
	}

	void SpriteRenderer::initRenderData() {
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

	void SpriteRenderer::setupShader(int texture_width, int texture_height, const RenderOptions &options) {
		glm::mat4 model = glm::mat4(1.);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(options.x * 16.f - options.xOffset * 2. * options.scaleX, options.y * 16.f - options.yOffset * 2. * options.scaleY, 0.0f));
		if (options.invertY)
			model = glm::scale(model, glm::vec3(1., -1., 1.));
		model = glm::translate(model, glm::vec3(.5 * texture_width, .5 * texture_height, 0.0f));
		model = glm::rotate   (model, float(glm::radians(options.angle)), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-.5 * texture_width, -.5 * texture_height, 0.0f));
		model = glm::scale    (model, glm::vec3(texture_width * options.scaleX, texture_height * options.scaleY, 1.0f));

		shader.bind();
		shader.set("model", model);
		shader.set("spriteColor", 1., 1., 1., options.alpha);
		const double multiplier = 2.;
		const double multiplier_x = multiplier / texture_width;
		const double multiplier_y = multiplier / texture_height;
		shader.set("texturePosition", options.xOffset * multiplier_x, options.yOffset * multiplier_y, options.sizeX / texture_width, options.sizeY / texture_height);
		// shader.set("divisor", divisor);
	}

	void SpriteRenderer::hackY(double &y, double yOffset, double scale) {
		y = backbufferHeight / 16. - y + yOffset / 4. * scale; // Four?!
	}
}
