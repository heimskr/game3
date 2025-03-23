#include "util/Log.h"
#include "graphics/GL.h"
#include "util/Debug.h"

namespace GL {
	void checkGL(GLenum err, const char *file, int line) {
		if (err) {
#ifdef __MINGW32__
			Game3::ERR("An OpenGL error occurred but you're on Windows so you don't get to know what it is.");
#else
			Game3::ERR("\x1b[31mError at {}:{}: {}\x1b[39m", file, line, reinterpret_cast<const char *>(gluErrorString(err)));
#endif
			Game3::Break();
		}
	}

	void VBO::update(const void *data, GLsizeiptr size, bool sub, GLenum usage) {
		if (bind()) {
			if (sub) {
				glBufferSubData(GL_ARRAY_BUFFER, 0, size, data); CHECKGL
			} else {
				glBufferData(GL_ARRAY_BUFFER, size, data, usage); CHECKGL
			}
		}
	}

	FBOBinder FBO::getBinder() {
		return FBOBinder(*this);
	}

	TextureFBOBinder Texture::getBinder() {
		return TextureFBOBinder(*this);
	}

	GLuint makeFloatVAO(GLuint vbo, std::initializer_list<int> sizes) {
		GLuint vao = -1;

		glGenVertexArrays(1, &vao); CHECKGL
		if (vao == static_cast<GLuint>(-1))
			throw std::runtime_error("Couldn't generate float VAO");

		glBindVertexArray(vao); CHECKGL
		glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECKGL

		int offset = 0;
		const auto stride = static_cast<GLsizei>(sizeof(float) * std::accumulate(sizes.begin(), sizes.end(), 0));

		for (size_t i = 0, n = sizes.size(); i < n; ++i)  {
			glEnableVertexAttribArray(i); CHECKGL
			glVertexAttribPointer(i, sizes.begin()[i], GL_FLOAT, GL_FALSE, stride, reinterpret_cast<GLvoid *>(sizeof(float) * offset)); CHECKGL
			offset += sizes.begin()[i];
		}

		return vao;
	}

	void TextureFBOBinder::save() {
		static_assert(sizeof(GLuint) == sizeof(GLint));
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, reinterpret_cast<GLint *>(&handle)); CHECKGL
	}

	void TextureFBOBinder::restore() {
		if (handle == 0)
			unbindFBTexture();
		else
			useTextureInFB(handle);
	}

	Scissor::Scissor() {
		glGetIntegerv(GL_SCISSOR_BOX, saved); CHECKGL
	}

	Scissor::Scissor(GLint x, GLint y, GLsizei width, GLsizei height) {
		glGetIntegerv(GL_SCISSOR_BOX, saved); CHECKGL
		reframe(x, y, width, height);
	}

	void Scissor::reframe(GLint x, GLint y, GLsizei width, GLsizei height) {
		Game3::INFO("Scissoring to {}, {}, {}, {}", x, y, width, height);
		glScissor(x, y, width, height); CHECKGL
	}

	void Scissor::reset() {
		glScissor(saved[0], saved[1], static_cast<GLsizei>(saved[2]), static_cast<GLsizei>(saved[3])); CHECKGL
	}
}
