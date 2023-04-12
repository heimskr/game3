#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "resources.h"
#include "Shader.h"
#include "ui/Reshader.h"
#include "util/GL.h"
#include "util/Util.h"

namespace Game3 {
	Reshader::Reshader(std::string_view fragment): shader("reshader") {
		shader.init(reshader_vert, fragment); CHECKGL
		initRenderData(); CHECKGL
	}

	Reshader::~Reshader() {
		reset();
	}

	void Reshader::reset() {
		if (initialized) {
			glDeleteBuffers(1, &vbo);
			glDeleteVertexArrays(1, &quadVAO); CHECKGL
			initialized = false;
		}
	}

	void Reshader::update(int backbuffer_width, int backbuffer_height) {
		if (backbuffer_width != backbufferWidth || backbuffer_height != backbufferHeight) {
			backbufferWidth = backbuffer_width;
			backbufferHeight = backbuffer_height;
			glm::mat4 projection = glm::ortho(0.f, float(backbuffer_width), float(backbuffer_height), 0.f, -1.f, 1.f);
			shader.bind(); CHECKGL
			shader.set("projection", projection); CHECKGL
		}
	}

	void Reshader::bind() {
		shader.bind(); CHECKGL
	}

	void Reshader::operator()(GLuint texture) {
		if (!initialized)
			return;

		shader.bind(); CHECKGL

		constexpr float x = 0.f;
		constexpr float y = 0.f;
		const float width  = backbufferWidth;
		const float height = backbufferHeight;

		glm::mat4 model = glm::mat4(1.f);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x, y, 0.f));
		model = glm::translate(model, glm::vec3(0.5f * width, 0.5f * height, 0.f)); // move origin of rotation to center of quad
		model = glm::rotate(model, glm::radians(0.f), glm::vec3(0.f, 0.f, 1.f)); // then rotate
		model = glm::translate(model, glm::vec3(-0.5f * width, -0.5f * height, 0.f)); // move origin back
		model = glm::scale(model, glm::vec3(width, height, 1.f)); // last scale

		shader.set("model", model);

		glActiveTexture(GL_TEXTURE4); CHECKGL
		glBindTexture(GL_TEXTURE_2D, texture); CHECKGL
		shader.set("txr", 4); CHECKGL

		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	void Reshader::operator()(const GL::Texture &texture) {
		assert(texture.getHandle() != 0);
		(*this)(texture.getHandle());
	}

	void Reshader::initRenderData() {
		static const float vertices[] {
			0.f, 1.f, 0.f, 1.f,
			1.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 0.f,

			0.f, 1.f, 0.f, 1.f,
			1.f, 1.f, 1.f, 1.f,
			1.f, 0.f, 1.f, 0.f,
		};

		glGenVertexArrays(1, &quadVAO); CHECKGL
		glGenBuffers(1, &vbo); CHECKGL

		GLint old_abb = 0;
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old_abb); CHECKGL

		glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); CHECKGL

		glBindVertexArray(quadVAO); CHECKGL
		glEnableVertexAttribArray(0); CHECKGL
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0); CHECKGL
		glBindBuffer(GL_ARRAY_BUFFER, old_abb); CHECKGL
		glBindVertexArray(0); CHECKGL
		initialized = true;
	}
}
