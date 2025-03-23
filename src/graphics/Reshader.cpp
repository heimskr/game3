#include "util/Log.h"
#include "graphics/Shader.h"
#include "graphics/GL.h"
#include "graphics/RenderOptions.h"
#include "graphics/Reshader.h"
#include "graphics/Texture.h"
#include "graphics/Util.h"
#include "util/FS.h"
#include "util/Util.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

namespace Game3 {
	namespace {
		const std::string & reshaderVert() { static auto out = readFile("resources/sprite.vert"); return out; }
	}

	Reshader::Reshader(std::string_view fragment_shader): shader("reshader") {
		shader.init(reshaderVert(), fragment_shader); CHECKGL
		initRenderData(); CHECKGL
	}

	Reshader::Reshader(std::string_view vertex_shader, std::string_view fragment_shader): shader("reshader") {
		shader.init(vertex_shader, fragment_shader); CHECKGL
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
			glm::mat4 projection = glm::ortho(0.f, static_cast<float>(backbuffer_width), static_cast<float>(backbuffer_height), 0.f, -1.f, 1.f);
			shader.bind(); CHECKGL
			shader.set("projection", projection); CHECKGL
		}
	}

	void Reshader::bind() {
		shader.bind();
	}

	bool Reshader::drawOnScreen(GLuint texture) {
		shader.bind();
		return draw(texture, glm::scale(glm::mat4(1.f), glm::vec3(backbufferWidth, backbufferHeight, 1.f)));
	}

	bool Reshader::drawOnScreen(const std::shared_ptr<Texture> &texture) {
		assert(texture != nullptr);
		assert(texture->id != 0);
		return drawOnScreen(texture->id);
	}

	bool Reshader::drawOnScreen(const GL::Texture &texture) {
		assert(texture.getHandle() != 0);
		return drawOnScreen(texture.getHandle());
	}

	bool Reshader::drawOnMap(const TexturePtr &texture, const RenderOptions &options, const Tileset &tileset, const Window &window) {\
		assert(texture != nullptr);
		return drawOnMap(texture->id, texture->width, texture->height, options, tileset, window);
	}

	bool Reshader::drawOnMap(int width, int height, const RenderOptions &options, const Tileset &tileset, const Window &window) {\
		return drawOnMap(0, width, height, options, tileset, window);
	}

	bool Reshader::drawOnMap(const GL::Texture &texture, const RenderOptions &options, const Tileset &tileset, const Window &window) {
		return drawOnMap(texture.getHandle(), texture.getWidth(), texture.getHeight(), options, tileset, window);
	}

	void Reshader::initRenderData() {
		if (initialized) {
			glDeleteVertexArrays(1, &quadVAO); CHECKGL
		}

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
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr); CHECKGL
		glBindBuffer(GL_ARRAY_BUFFER, old_abb); CHECKGL_SET
		if (gl_err)
			WARN("old_abb = {}", old_abb);
		glBindVertexArray(0); CHECKGL
		initialized = true;
	}

	bool Reshader::draw(GLuint texture, const glm::mat4 &model) {
		if (!initialized) {
			return false;
		}

		shader.set("model", model);

		if (texture > 0) {
			glActiveTexture(GL_TEXTURE1); CHECKGL
			glBindTexture(GL_TEXTURE_2D, texture); CHECKGL
			shader.set("txr", 1); CHECKGL
		}

		if (shaderSetup) {
			shaderSetup(shader, texture);
		}

		glBindVertexArray(quadVAO); CHECKGL
		glDrawArrays(GL_TRIANGLES, 0, 6); CHECKGL
		glBindVertexArray(0); CHECKGL

		return true;
	}

	bool Reshader::drawOnMap(GLuint texture, int texture_width, int texture_height, const RenderOptions &options, const Tileset &tileset, const Window &window) {
		shader.bind();
		return draw(texture, makeMapModel(options, texture_width, texture_height, tileset, window));
	}
}
