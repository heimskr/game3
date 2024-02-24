#include "Log.h"
#include "graphics/Shader.h"
#include "graphics/GL.h"
#include "graphics/Reshader.h"
#include "util/FS.h"
#include "util/Util.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

namespace Game3 {
	namespace {
		const std::string & reshaderVert() { static auto out = readFile("resources/reshader.vert"); return out; }
	}

	Reshader::Reshader(std::string_view fragment): shader("reshader") {
		shader.init(reshaderVert(), fragment); CHECKGL
		initRenderData(); CHECKGL
	}

	Reshader::~Reshader() {
		reset();
	}

	void Reshader::reset() {
		if (initialized) {
			glDeleteBuffers(1, &vbo); CHECKGL
			glDeleteVertexArrays(1, &quadVAO); CHECKGL
			vbo = 0;
			quadVAO = 0;
			shader.reset();
			initialized = false;
		}
	}

	void Reshader::update(int backbuffer_width, int backbuffer_height) {
		if (backbuffer_width != backbufferWidth || backbuffer_height != backbufferHeight) {
			backbufferWidth = backbuffer_width;
			backbufferHeight = backbuffer_height;
			glm::dmat4 projection = glm::ortho(0., double(backbuffer_width), double(backbuffer_height), 0., -1., 1.);
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

		const double width(backbufferWidth);
		const double height(backbufferHeight);

		glm::dmat4 model = glm::scale(glm::dmat4(1.), glm::dvec3(width, height, 1.));

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
			0, 1, 0, 1,
			1, 0, 1, 0,
			0, 0, 0, 0,

			0, 1, 0, 1,
			1, 1, 1, 1,
			1, 0, 1, 0,
		};

		glGenVertexArrays(1, &quadVAO); CHECKGL
		glGenBuffers(1, &vbo); CHECKGL

		GLint old_abb = 0;
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old_abb); CHECKGL

		glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); CHECKGL

		glBindVertexArray(quadVAO); CHECKGL
		glEnableVertexAttribArray(0); CHECKGL
		GL::vertexAttribPointer<float>(0, 4, GL_FALSE, 4 * sizeof(float), nullptr); CHECKGL
		glBindBuffer(GL_ARRAY_BUFFER, old_abb); CHECKGL_SET
		if (gl_err)
			WARN_("old_abb = " << old_abb);
		glBindVertexArray(0); CHECKGL
		initialized = true;
	}
}
