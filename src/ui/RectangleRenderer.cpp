#include <iostream>

#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>

#include "resources.h"
#include "ui/RectangleRenderer.h"
#include "util/Util.h"

namespace Game3 {
	RectangleRenderer::RectangleRenderer(): shader("RectangleRenderer") {
		shader.init(rectangle_vert, rectangle_frag); CHECKGL
		initRenderData(); CHECKGL
	}

	RectangleRenderer::~RectangleRenderer() {
		if (initialized) {
			glDeleteVertexArrays(1, &quadVAO); CHECKGL
		}
	}

	void RectangleRenderer::update(int backbuffer_width, int backbuffer_height) {
		if (backbuffer_width != backbufferWidth || backbuffer_height != backbufferHeight) {
			backbufferWidth = backbuffer_width;
			backbufferHeight = backbuffer_height;
			glm::mat4 projection = glm::ortho(0.f, float(backbuffer_width), float(backbuffer_height), 0.f, -1.f, 1.f);
			shader.bind(); CHECKGL
			shader.set("projection", projection); CHECKGL
		}
	}

	void RectangleRenderer::drawOnScreen(const Eigen::Vector4f &color, float x, float y, float width, float height, float angle) {
		if (!initialized)
			return;

		shader.bind(); CHECKGL

		glm::mat4 model = glm::mat4(1.f);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x, y, 0.f));
		model = glm::translate(model, glm::vec3(0.5f * width, 0.5f * height, 0.f)); // move origin of rotation to center of quad
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.f, 0.f, 1.f)); // then rotate
		model = glm::translate(model, glm::vec3(-0.5f * width, -0.5f * height, 0.f)); // move origin back
		model = glm::scale(model, glm::vec3(width, height, 2.f)); // last scale

		shader.set("model", model); CHECKGL
		shader.set("rectColor", color); CHECKGL

		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	void RectangleRenderer::operator()(const Eigen::Vector4f &color, float x, float y, float width, float height, float angle) {
		drawOnScreen(color, x, y, width, height, angle);
	}

	void RectangleRenderer::initRenderData() {
		unsigned int vbo;
		static float vertices[] {
			0.f, 1.f, 0.f, 1.f,
			1.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 0.f,

			0.f, 1.f, 0.f, 1.f,
			1.f, 1.f, 1.f, 1.f,
			1.f, 0.f, 1.f, 0.f,
		};

		CHECKGL

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
