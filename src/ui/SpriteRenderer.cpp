// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite_renderer.cpp

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Texture.h"
#include "game/Entity.h"
#include "game/Game.h"
#include "ui/Canvas.h"
#include "ui/SpriteRenderer.h"
#include "util/Util.h"

#include <GL/glu.h>

namespace Game3 {
	SpriteRenderer::SpriteRenderer(Canvas &canvas_): canvas(canvas_) {
		shader.initFromFiles("SpriteRenderer", "resources/sprite.vert", "resources/sprite.frag");
		initRenderData();
	}

	SpriteRenderer::~SpriteRenderer() {
		shader.free();
		if (initialized)
			glDeleteVertexArrays(1, &quadVAO);
	}

	void SpriteRenderer::update(int backbuffer_width, int backbuffer_height) {
		if (backbuffer_width != backbufferWidth || backbuffer_height != backbufferHeight) {
			backbufferWidth = backbuffer_width;
			backbufferHeight = backbuffer_height;
			glm::mat4 projection = glm::ortho(0.f, float(backbuffer_width), float(backbuffer_height), 0.f, -1.f, 1.f);
			shader.bind();
			glUniformMatrix4fv(shader.uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection)); CHECKGL
		}
	}

	void SpriteRenderer::draw(Texture &texture, float x, float y, float scale, float angle, float alpha) {
		draw(texture, x, y, 0, 0, texture.width, texture.height, scale, angle, alpha);
	}

	void SpriteRenderer::draw(Texture &texture, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale, float angle, float alpha) {
		if (!initialized)
			return;

		if (size_x < 0)
			size_x = texture.width;
		if (size_y < 0)
			size_y = texture.height;


		const auto &tilemap = canvas.game->activeRealm->tilemap1;
		x *= tilemap->tileSize * canvas.scale / 2.f;
		y *= tilemap->tileSize * canvas.scale / 2.f;

		x += canvas.width() / 2.f;
		x -= tilemap->width * tilemap->tileSize * canvas.scale / canvas.magic * 2.f; // TODO: the math here is a little sus... things might cancel out
		x += canvas.center.x() * canvas.scale * tilemap->tileSize / 2.f;

		y += canvas.height() / 2.f;
		y -= tilemap->height * tilemap->tileSize * canvas.scale / canvas.magic * 2.f;
		y += canvas.center.y() * canvas.scale * tilemap->tileSize / 2.f;

		size_x *= canvas.scale / 2.f;
		size_y *= canvas.scale / 2.f;
		scale *= canvas.scale / 2.f;
		x_offset *= canvas.scale;
		y_offset *= canvas.scale;

		shader.bind();

		glm::mat4 model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(x - x_offset, y - y_offset, 0.f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(0.5f * texture.width, 0.5f * texture.height, 0.f)); // move origin of rotation to center of quad
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.f, 0.f, 1.f)); // then rotate
		model = glm::translate(model, glm::vec3(-0.5f * texture.width, -0.5f * texture.height, 0.f)); // move origin back
		model = glm::scale(model, glm::vec3(texture.width * scale, texture.height * scale, 2.f)); // last scale

		glUniformMatrix4fv(shader.uniform("model"), 1, GL_FALSE, glm::value_ptr(model)); CHECKGL
		glUniform3f(shader.uniform("spriteColor"), 1.f, 1.f, 1.f); CHECKGL

		glActiveTexture(GL_TEXTURE0); CHECKGL
		texture.bind(); CHECKGL

		glEnable(GL_SCISSOR_TEST);
		glScissor(2 * x, 2 * (backbufferHeight - y - size_y), 2 * size_x, 2 * size_y);
		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
		glDisable(GL_SCISSOR_TEST);
	}

	void SpriteRenderer::initRenderData() {
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
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		initialized = true;
	}
}
