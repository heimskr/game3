// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite_renderer.cpp

#include <iostream>

#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Texture.h"
#include "Tileset.h"
#include "resources.h"
#include "game/ClientGame.h"
#include "ui/Canvas.h"
#include "ui/SpriteRenderer.h"
#include "util/GL.h"
#include "util/Util.h"

namespace Game3 {
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
			glm::mat4 projection = glm::ortho(0.f, float(backbuffer_width), float(backbuffer_height), 0.f, -1.f, 1.f);
			shader.bind();
			shader.set("projection", projection);
		}
	}

	void SpriteRenderer::drawOnMap(Texture &texture, float x, float y, float scale, float angle, float alpha) {
		drawOnMap(texture, RenderOptions {
			.x = x,
			.y = y,
			.size_x = static_cast<float>(*texture.width),
			.size_y = static_cast<float>(*texture.height),
			.scaleX = scale,
			.scaleY = scale,
			.angle = angle,
			.alpha = alpha
		});
	}

	void SpriteRenderer::drawOnMap(Texture &texture, RenderOptions options) {
		if (!initialized)
			return;

		if (options.size_x < 0)
			options.size_x = *texture.width;
		if (options.size_y < 0)
			options.size_y = *texture.height;

		auto &provider = canvas->game->activeRealm->tileProvider;
		const auto &tileset   = provider.getTileset(*canvas->game);
		const auto tile_size  = tileset->getTileSize();
		const auto map_length = CHUNK_SIZE * REALM_DIAMETER;

		options.x *= tile_size * canvas->scale / 2.f;
		options.y *= tile_size * canvas->scale / 2.f;

		options.x += canvas->width() / 2.f;
		options.x -= map_length * tile_size * canvas->scale / canvas->magic * 2.f; // TODO: the math here is a little sus... things might cancel out
		options.x += centerX * canvas->scale * tile_size / 2.f;

		options.y += canvas->height() / 2.f;
		options.y -= map_length * tile_size * canvas->scale / canvas->magic * 2.f;
		options.y += centerY * canvas->scale * tile_size / 2.f;

		shader.bind();

		glm::mat4 model = glm::mat4(1.f);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(options.x - options.x_offset * canvas->scale * options.scaleX, options.y - options.y_offset * canvas->scale * options.scaleY, 0.f));
		model = glm::translate(model, glm::vec3(0.5f * *texture.width, 0.5f * *texture.height, 0.f)); // move origin of rotation to center of quad
		model = glm::rotate(model, glm::radians(options.angle), glm::vec3(0.f, 0.f, 1.f)); // then rotate
		model = glm::translate(model, glm::vec3(-0.5f * *texture.width, -0.5f * *texture.height, 0.f)); // move origin back
		model = glm::scale(model, glm::vec3(*texture.width * options.scaleX * canvas->scale / 2.f, *texture.height * options.scaleY * canvas->scale / 2.f, 2.f)); // last scale

		shader.set("model", model);
		shader.set("spriteColor", 1.f, 1.f, 1.f, options.alpha);
		const float multiplier = 2.f / *texture.width;
		shader.set("texturePosition", options.x_offset * multiplier, options.y_offset * multiplier, options.size_x / *texture.width, options.size_y / *texture.width);
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

		if (options.size_x < 0)
			options.size_x = texture_width;
		if (options.size_y < 0)
			options.size_y = texture_height;

		auto &provider = canvas->game->activeRealm->tileProvider;
		const auto &tileset   = provider.getTileset(*canvas->game);
		const auto tile_size  = tileset->getTileSize();
		const auto map_length = CHUNK_SIZE * REALM_DIAMETER;

		options.x *= tile_size * canvas->scale / 2.f;
		options.y *= tile_size * canvas->scale / 2.f;

		options.x += canvas->width() / 2.f;
		options.x -= map_length * tile_size * canvas->scale / canvas->magic * 2.f; // TODO: the math here is a little sus... things might cancel out
		options.x += centerX * canvas->scale * tile_size / 2.f;

		options.y += canvas->height() / 2.f;
		options.y -= map_length * tile_size * canvas->scale / canvas->magic * 2.f;
		options.y += centerY * canvas->scale * tile_size / 2.f;

		shader.bind();

		glm::mat4 model = glm::mat4(1.f);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(options.x - options.x_offset * canvas->scale * options.scaleX, options.y - options.y_offset * canvas->scale * options.scaleY, 0.f));
		model = glm::translate(model, glm::vec3(0.5f * texture_width, 0.5f * texture_height, 0.f)); // move origin of rotation to center of quad
		model = glm::rotate(model, glm::radians(options.angle), glm::vec3(0.f, 0.f, 1.f)); // then rotate
		model = glm::translate(model, glm::vec3(-0.5f * texture_width, -0.5f * texture_height, 0.f)); // move origin back
		model = glm::scale(model, glm::vec3(texture_width * options.scaleX * canvas->scale / 2.f, texture_height * options.scaleY * canvas->scale / 2.f, 2.f)); // last scale

		shader.set("model", model);
		shader.set("spriteColor", 1.f, 1.f, 1.f, options.alpha);
		const float multiplier = 2.f / texture_width;
		shader.set("texturePosition", options.x_offset * multiplier, options.y_offset * multiplier, options.size_x / texture_width, options.size_y / texture_width);
		// shader.set("divisor", divisor);

		texture.bind(0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	void SpriteRenderer::drawOnScreen(Texture &texture, float x, float y, float scale, float angle, float alpha) {
		drawOnScreen(texture, RenderOptions {
			.x = x,
			.y = y,
			.size_x = static_cast<float>(*texture.width),
			.size_y = static_cast<float>(*texture.height),
			.scaleX = scale,
			.scaleY = scale,
			.angle = angle,
			.alpha = alpha
		});
	}

	void SpriteRenderer::drawOnScreen(Texture &texture, RenderOptions options) {
		if (!initialized)
			return;

		const auto texture_width  = *texture.width;
		const auto texture_height = *texture.height;
		if (options.hackY)
			hackY(options.y, options.y_offset, options.scaleY);

		options.size_x = options.size_x < 0.f? -options.size_x * texture_width  : options.size_x;
		options.size_y = options.size_y < 0.f? -options.size_y * texture_height : options.size_y;
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
			hackY(options.y, options.y_offset, options.scaleY);

		options.size_x = options.size_x < 0.f? -options.size_x * texture_width  : options.size_x;
		options.size_y = options.size_y < 0.f? -options.size_y * texture_height : options.size_y;
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
			0.f, 1.f, 0.f, 1.f,
			1.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 0.f,

			0.f, 1.f, 0.f, 1.f,
			1.f, 1.f, 1.f, 1.f,
			1.f, 0.f, 1.f, 0.f
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
		glm::mat4 model = glm::mat4(1.f);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(options.x * 16.f - options.x_offset * 2.f * options.scaleX, options.y * 16.f - options.y_offset * 2.f * options.scaleY, 0.0f));
		if (options.invertY)
			model = glm::scale(model, glm::vec3(1.f, -1.f, 1.f));
		model = glm::translate(model, glm::vec3(0.5f * texture_width, 0.5f * texture_height, 0.0f));
		model = glm::rotate   (model, glm::radians(options.angle), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-0.5f * texture_width, -0.5f * texture_height, 0.0f));
		model = glm::scale    (model, glm::vec3(texture_width * options.scaleX, texture_height * options.scaleY, 1.0f));

		shader.bind();
		shader.set("model", model);
		shader.set("spriteColor", 1.f, 1.f, 1.f, options.alpha);
		const float multiplier = 2.f;
		const float multiplier_x = multiplier / texture_width;
		const float multiplier_y = multiplier / texture_height;
		shader.set("texturePosition", options.x_offset * multiplier_x, options.y_offset * multiplier_y, options.size_x / texture_width, options.size_y / texture_height);
		// shader.set("divisor", divisor);
	}

	void SpriteRenderer::hackY(float &y, float y_offset, float scale) {
		y = backbufferHeight / 16.f - y + y_offset / 4.f * scale; // Four?!
	}
}
