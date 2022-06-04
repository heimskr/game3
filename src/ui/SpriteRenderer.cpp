// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite_renderer.cpp

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Texture.h"
#include "ui/SpriteRenderer.h"
#include "util/Util.h"

#include <GL/glu.h>

namespace Game3 {
	SpriteRenderer::SpriteRenderer() {
		shader.initFromFiles("SpriteRenderer", "resources/sprite.vert", "resources/sprite.frag");
		initRenderData();
	}

	SpriteRenderer::~SpriteRenderer() {
		shader.free();
		if (initialized)
			glDeleteVertexArrays(1, &quadVAO);
	}

	void SpriteRenderer::draw(Texture &texture, float x, float y, float scale, float angle, float alpha) {
		draw(texture, x, y, 0, 0, texture.width, texture.height, scale, angle, alpha);
	}

	void SpriteRenderer::draw(Texture &texture, float x, float y, float x_offset, float y_offset, float size_x, float size_y, float scale, float angle, float alpha) {
		if (!initialized)
			return;

		// auto pattern = nvgImagePattern(context_, x + x_offset, y + y_offset, width_ * scale, height_ * scale, angle, nvg_, alpha);
		// nvgRect(context_, x, y, size_x * scale, size_y * scale);

		if (size_x < 0)
			size_x = texture.width;
		if (size_y < 0)
			size_y = texture.height;

		shader.bind();

		glm::mat4 model = glm::mat4(1.f);
		model = glm::translate(model, glm::vec3(x, y, 0.f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(0.5f * size_x, 0.5f * size_y, 0.f)); // move origin of rotation to center of quad
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.f, 0.f, 1.f)); // then rotate
		model = glm::translate(model, glm::vec3(-0.5f * size_x, -0.5f * size_y, 0.f)); // move origin back
		model = glm::scale(model, glm::vec3(size_x * scale, size_y * scale, 1.f)); // last scale

		glUniformMatrix4fv(shader.uniform("model"), 1, false, glm::value_ptr(model));CHECKGL
		// glUniform2f(shader.uniform("offset"), x_offset, y_offset);CHECKGL
		glUniform3f(shader.uniform("spriteColor"), 1.f, 1.f, 1.f);CHECKGL

		glActiveTexture(GL_TEXTURE0);CHECKGL
		texture.bind();CHECKGL

		glBindVertexArray(quadVAO);CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6);CHECKGL
		glBindVertexArray(0);CHECKGL
	}

	void SpriteRenderer::initRenderData() {
		unsigned int vbo;
		static float vertices[] {
			// pos      // tex
			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,

			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f
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
