#include <nanogui/opengl.h>
#include <nanogui/glutil.h>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glu.h>

#include "ui/RectangleRenderer.h"
#include "util/Util.h"

namespace Game3 {
	RectangleRenderer::RectangleRenderer(Canvas &canvas_): canvas(canvas_) {
		shader.initFromFiles("RectangleRenderer", "resources/rect.vert", "resources/rect.frag");
		initRenderData();
	}

	RectangleRenderer::~RectangleRenderer() {
		shader.free();
		if (initialized)
			glDeleteVertexArrays(1, &quadVAO);
	}

	void RectangleRenderer::update(int backbuffer_width, int backbuffer_height) {
		if (backbuffer_width != backbufferWidth || backbuffer_height != backbufferHeight) {
			backbufferWidth = backbuffer_width;
			backbufferHeight = backbuffer_height;
			glm::mat4 projection = glm::ortho(0.f, float(backbuffer_width), float(backbuffer_height), 0.f, -1.f, 1.f);
			shader.bind();
			glUniformMatrix4fv(shader.uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection)); CHECKGL
		}
	}

	void RectangleRenderer::drawOnScreen(const nanogui::Vector4f &color, float x, float y, float width, float height, float angle) {
		if (!initialized)
			return;

		shader.bind();

		glm::mat4 model = glm::mat4(1.f);
		// first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
		model = glm::translate(model, glm::vec3(x, y, 0.f));
		model = glm::translate(model, glm::vec3(0.5f * width, 0.5f * height, 0.f)); // move origin of rotation to center of quad
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.f, 0.f, 1.f)); // then rotate
		model = glm::translate(model, glm::vec3(-0.5f * width, -0.5f * height, 0.f)); // move origin back
		model = glm::scale(model, glm::vec3(width, height, 2.f)); // last scale

		glUniformMatrix4fv(shader.uniform("model"), 1, GL_FALSE, glm::value_ptr(model)); CHECKGL
		glUniform4f(shader.uniform("rectColor"), color.x(), color.y(), color.z(), color.w()); CHECKGL

		// glActiveTexture(GL_TEXTURE0); CHECKGL
		// texture.bind(); CHECKGL

		// glEnable(GL_SCISSOR_TEST);
		// glScissor(2 * x, 2 * (backbufferHeight - y - height), 2 * width, 2 * height);
		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
		// glDisable(GL_SCISSOR_TEST);
	}

	void RectangleRenderer::operator()(const nanogui::Vector4f &color, float x, float y, float width, float height, float angle) {
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
			1.f, 0.f, 1.f, 0.f
		};

		glGenVertexArrays(1, &quadVAO); CHECKGL
		glGenBuffers(1, &vbo); CHECKGL

		glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); CHECKGL

		glBindVertexArray(quadVAO); CHECKGL
		glEnableVertexAttribArray(0); CHECKGL
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0); CHECKGL
		glBindBuffer(GL_ARRAY_BUFFER, 0); CHECKGL
		glBindVertexArray(0); CHECKGL
		initialized = true;
	}
}
