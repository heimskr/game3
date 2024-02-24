#include "Log.h"
#include "graphics/GL.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/TextureCombiner.h"
#include "util/Util.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

namespace Game3 {
	TextureCombiner::TextureCombiner(const std::string &name, const std::string &vertex, const std::string &fragment): shader(name) {
		INFO("Initialized TextureCombiner \"{}\"; shader name is \"{}\"", name, shader.getName());
		shader.init(vertex, fragment); CHECKGL
		INFO("Shader handle is {}", shader.getHandle());
		initRenderData(); CHECKGL
	}

	TextureCombiner::~TextureCombiner() {
		reset();
	}

	void TextureCombiner::reset() {
		if (!initialized)
			return;

		glDeleteBuffers(1, &vbo); CHECKGL
		glDeleteVertexArrays(1, &quadVAO); CHECKGL
		vbo = 0;
		quadVAO = 0;
		shader.reset();
		initialized = false;
	}

	void TextureCombiner::update(int backbuffer_width, int backbuffer_height) {
		if (backbuffer_width != backbufferWidth || backbuffer_height != backbufferHeight) {
			backbufferWidth = backbuffer_width;
			backbufferHeight = backbuffer_height;
			projection = glm::ortho(0., double(backbuffer_width), double(backbuffer_height), 0., -1., 1.);
			shader.bind();
			shader.set("projection", projection);
		}
	}

	void TextureCombiner::bind() {
		shader.bind(); CHECKGL
	}

	void TextureCombiner::operator()(GLuint texture0, GLuint texture1) {
		if (!initialized)
			return;

		assert(texture0 != 0);
		assert(texture1 != 0);
		shader.bind(); CHECKGL

		glm::dmat4 model(1.);
		model = glm::scale(model, glm::dvec3(1., -1., 1.));
		model = glm::translate(model, glm::dvec3(0, -backbufferHeight, 0.));
		model = glm::scale(model, glm::dvec3(backbufferWidth, backbufferHeight, 1.));

		shader.set("model", model);

		glActiveTexture(GL_TEXTURE5); CHECKGL
		glBindTexture(GL_TEXTURE_2D, texture0); CHECKGL
		shader.set("texture0", 5);

		glActiveTexture(GL_TEXTURE6); CHECKGL
		glBindTexture(GL_TEXTURE_2D, texture1); CHECKGL
		shader.set("texture1", 6);

		glEnable(GL_BLEND); CHECKGL
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); CHECKGL
		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL
	}

	void TextureCombiner::operator()(const GL::Texture &texture0, const GL::Texture &texture1) {
		(*this)(texture0.getHandle(), texture1.getHandle());
	}

	void TextureCombiner::operator()(const Texture &texture0, const Texture &texture1) {
		(*this)(texture0.id, texture1.id);
	}

	void TextureCombiner::initRenderData() {
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

		glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); CHECKGL

		glBindVertexArray(quadVAO); CHECKGL
		glEnableVertexAttribArray(0); CHECKGL
		GL::vertexAttribPointer<float>(0, 4, GL_FALSE, 4 * sizeof(float), nullptr); CHECKGL
		glBindBuffer(GL_ARRAY_BUFFER, 0); CHECKGL
		glBindVertexArray(0); CHECKGL
		initialized = true;
	}
}
