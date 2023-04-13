// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite_renderer.cpp

#include <iostream>

#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Texture.h"
#include "game/Game.h"
#include "ui/Canvas.h"
#include "ui/SpriteRenderer.h"
#include "resources.h"
#include "util/GL.h"
#include "util/Util.h"

namespace Game3 {
	SpriteRenderer::SpriteRenderer(Canvas &canvas_): canvas(&canvas_), shader("SpriteRenderer") {
		shader.init(sprite_vert, sprite_frag);
		initRenderData();
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
		drawOnMap(texture, x, y, 0, 0, *texture.width, *texture.height, scale, angle, alpha);
	}

	void SpriteRenderer::drawOnMap(Texture &texture, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale, float angle, float alpha) {
		if (!initialized)
			return;

		if (size_x < 0)
			size_x = *texture.width;
		if (size_y < 0)
			size_y = *texture.height;

		const auto &tilemap = canvas->game->activeRealm->tilemap1;
		x *= tilemap->tileSize * canvas->scale / 2.f;
		y *= tilemap->tileSize * canvas->scale / 2.f;

		x += canvas->width() / 2.f;
		x -= tilemap->width * tilemap->tileSize * canvas->scale / canvas->magic * 2.f; // TODO: the math here is a little sus... things might cancel out
		x += canvas->center.x() * canvas->scale * tilemap->tileSize / 2.f;

		y += canvas->height() / 2.f;
		y -= tilemap->height * tilemap->tileSize * canvas->scale / canvas->magic * 2.f;
		y += canvas->center.y() * canvas->scale * tilemap->tileSize / 2.f;

		shader.bind();

		glm::mat4 model = glm::mat4(1.f);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x - x_offset * canvas->scale * scale, y - y_offset * canvas->scale * scale, 0.f));
		model = glm::translate(model, glm::vec3(0.5f * *texture.width, 0.5f * *texture.height, 0.f)); // move origin of rotation to center of quad
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.f, 0.f, 1.f)); // then rotate
		model = glm::translate(model, glm::vec3(-0.5f * *texture.width, -0.5f * *texture.height, 0.f)); // move origin back
		model = glm::scale(model, glm::vec3(*texture.width * scale * canvas->scale / 2.f, *texture.height * scale * canvas->scale / 2.f, 2.f)); // last scale

		shader.set("model", model);
		shader.set("spriteColor", 1.f, 1.f, 1.f, alpha);
		const float multiplier = 2.f / *texture.width;
		shader.set("texturePosition", x_offset * multiplier, y_offset * multiplier, size_x / *texture.width, size_y / *texture.width);

		glActiveTexture(GL_TEXTURE0);
		texture.bind();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	void SpriteRenderer::drawOnMap(GL::Texture &texture, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale, float angle, float alpha) {
		if (!initialized)
			return;

		const auto twidth = texture.getWidth();
		const auto theight = texture.getHeight();

		if (size_x < 0)
			size_x = twidth;
		if (size_y < 0)
			size_y = theight;

		const auto &tilemap = canvas->game->activeRealm->tilemap1;
		x *= tilemap->tileSize * canvas->scale / 2.f;
		y *= tilemap->tileSize * canvas->scale / 2.f;

		x += canvas->width() / 2.f;
		x -= tilemap->width * tilemap->tileSize * canvas->scale / canvas->magic * 2.f; // TODO: the math here is a little sus... things might cancel out
		x += canvas->center.x() * canvas->scale * tilemap->tileSize / 2.f;

		y += canvas->height() / 2.f;
		y -= tilemap->height * tilemap->tileSize * canvas->scale / canvas->magic * 2.f;
		y += canvas->center.y() * canvas->scale * tilemap->tileSize / 2.f;

		shader.bind();

		glm::mat4 model = glm::mat4(1.f);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x - x_offset * canvas->scale * scale, y - y_offset * canvas->scale * scale, 0.f));
		model = glm::translate(model, glm::vec3(0.5f * twidth, 0.5f * theight, 0.f)); // move origin of rotation to center of quad
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.f, 0.f, 1.f)); // then rotate
		model = glm::translate(model, glm::vec3(-0.5f * twidth, -0.5f * theight, 0.f)); // move origin back
		model = glm::scale(model, glm::vec3(twidth * scale * canvas->scale / 2.f, theight * scale * canvas->scale / 2.f, 2.f)); // last scale

		shader.set("model", model);
		shader.set("spriteColor", 1.f, 1.f, 1.f, alpha);
		const float multiplier = 2.f / twidth;
		shader.set("texturePosition", x_offset * multiplier, y_offset * multiplier, size_x / twidth, size_y / twidth);

		texture.bind(0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	void SpriteRenderer::drawOnScreen(Texture &texture, float x, float y, float scale, float angle, float alpha) {
		drawOnScreen(texture, x, y, 0, 0, *texture.width, *texture.height, scale, angle, alpha);
	}

	void SpriteRenderer::drawOnScreen(Texture &texture, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale, float angle, float alpha) {
		if (!initialized)
			return;

		const auto twidth  = *texture.width;
		const auto theight = *texture.height;

		if (size_x < 0)
			size_x = twidth;
		if (size_y < 0)
			size_y = theight;

		shader.bind();

		y = backbufferHeight / 16.f - y + y_offset / 4.f * scale; // Four?!

		glm::mat4 model = glm::mat4(1.f);
		// // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x * 16.f - x_offset * 2.f * scale, y * 16.f - y_offset * 2.f * scale, 0.0f));
		model = glm::scale    (model, glm::vec3(1.f, -1.f, 1.f));
		model = glm::translate(model, glm::vec3(0.5f * twidth, 0.5f * theight, 0.0f));
		model = glm::rotate   (model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-0.5f * twidth, -0.5f * theight, 0.0f));
		model = glm::scale    (model, glm::vec3(twidth * scale, theight * scale, 1.0f));

		shader.set("model", model);
		shader.set("spriteColor", 1.f, 1.f, 1.f, alpha);
		const float multiplier = 2.f;
		const float multiplier_x = multiplier / twidth;
		const float multiplier_y = multiplier / theight;
		shader.set("texturePosition", x_offset * multiplier_x, y_offset * multiplier_y, size_x / twidth, size_y / theight);

		glActiveTexture(GL_TEXTURE0);
		texture.bind();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
	void SpriteRenderer::drawOnScreen(GL::Texture &texture, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale, float angle, float alpha) {
		if (!initialized)
			return;

		const auto twidth  = texture.getWidth();
		const auto theight = texture.getHeight();

		if (size_x < 0)
			size_x = twidth;
		if (size_y < 0)
			size_y = theight;

		shader.bind();

		y = backbufferHeight / 16.f - y + y_offset / 4.f * scale; // Four?!

		glm::mat4 model = glm::mat4(1.f);
		// // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x * 16.f - x_offset * 2.f * scale, y * 16.f - y_offset * 2.f * scale, 0.0f));
		model = glm::scale    (model, glm::vec3(1.f, -1.f, 1.f));
		model = glm::translate(model, glm::vec3(0.5f * twidth, 0.5f * theight, 0.0f));
		model = glm::rotate   (model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-0.5f * twidth, -0.5f * theight, 0.0f));
		model = glm::scale    (model, glm::vec3(twidth * scale, theight * scale, 1.0f));

		shader.set("model", model);
		shader.set("spriteColor", 1.f, 1.f, 1.f, alpha);
		const float multiplier = 2.f;
		const float multiplier_x = multiplier / twidth;
		const float multiplier_y = multiplier / theight;
		shader.set("texturePosition", x_offset * multiplier_x, y_offset * multiplier_y, size_x / twidth, size_y / theight);

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
		static float vertices[] {
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
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		initialized = true;
	}
}
