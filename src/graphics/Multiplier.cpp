#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/GL.h"
#include "graphics/Multiplier.h"
#include "util/FS.h"
#include "util/Util.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

namespace Game3 {
	namespace {
		const std::string & multiplierFrag() { static auto out = readFile("resources/multiplier.frag"); return out; }
		const std::string & multiplierVert() { static auto out = readFile("resources/multiplier.vert"); return out; }
	}

	Multiplier::Multiplier(): shader("multiplier") {
		shader.init(multiplierVert(), multiplierFrag()); CHECKGL
		initRenderData(); CHECKGL
	}

	Multiplier::~Multiplier() {
		reset();
	}

	void Multiplier::reset() {
		if (initialized) {
			glDeleteBuffers(1, &vbo); CHECKGL
			glDeleteVertexArrays(1, &quadVAO); CHECKGL
			vbo = 0;
			quadVAO = 0;
			shader.reset();
			initialized = false;
		}
	}

	void Multiplier::update(int backbuffer_width, int backbuffer_height) {
		if (backbuffer_width != backbufferWidth || backbuffer_height != backbufferHeight) {
			backbufferWidth = backbuffer_width;
			backbufferHeight = backbuffer_height;
			projection = glm::ortho(0.f, float(backbuffer_width), float(backbuffer_height), 0.f, -1.f, 1.f);
		}
	}

	void Multiplier::bind() {
		shader.bind(); CHECKGL
	}

	void Multiplier::operator()(GLuint texture0, GLuint texture1, float width, float height) {
		(void) width;
		(void) height;

		if (!initialized)
			return;

		assert(texture0 != 0);
		assert(texture1 != 0);
		shader.bind(); CHECKGL

		const float bb_width  = backbufferWidth;
		const float bb_height = backbufferHeight;

		// y = backbufferHeight - y;

		glm::mat4 model = glm::mat4(1.f);
		model = glm::scale(model, glm::vec3(1.f, -1.f, 1.f));
		model = glm::translate(model, glm::vec3(0, -bb_height, 0.f));
		model = glm::scale(model, glm::vec3(bb_width, bb_height, 1.f));

		shader.set("projection", projection);
		shader.set("model", model);

		glActiveTexture(GL_TEXTURE5); CHECKGL
		glBindTexture(GL_TEXTURE_2D, texture0); CHECKGL
		shader.set("texture0", 5);

		glActiveTexture(GL_TEXTURE6); CHECKGL
		glBindTexture(GL_TEXTURE_2D, texture1); CHECKGL
		shader.set("texture1", 6);

		// glEnable(GL_BLEND);
		// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	void Multiplier::operator()(const GL::Texture &texture0, const GL::Texture &texture1) {
		// assert(texture0.getWidth() == texture1.getWidth());
		// assert(texture0.getHeight() == texture1.getHeight());
		(*this)(texture0.getHandle(), texture1.getHandle(), texture0.getWidth(), texture0.getHeight());
	}

	void Multiplier::operator()(const Texture &texture0, const Texture &texture1) {
		// assert(*texture0.width == *texture1.width);
		// assert(*texture0.height == *texture1.height);
		(*this)(texture0.id, texture1.id, texture0.width, texture0.height);
	}

	void Multiplier::initRenderData() {
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

		// GLint old_abb = 0;
		// glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old_abb); CHECKGL

		glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); CHECKGL

		glBindVertexArray(quadVAO); CHECKGL
		glEnableVertexAttribArray(0); CHECKGL
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr); CHECKGL
		// glBindBuffer(GL_ARRAY_BUFFER, old_abb); CHECKGL
		glBindBuffer(GL_ARRAY_BUFFER, 0); CHECKGL
		glBindVertexArray(0); CHECKGL
		initialized = true;
	}
}
