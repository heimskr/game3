#include "Log.h"
#include "graphics/GL.h"

namespace GL {
	void checkGL(GLenum err, const char *file, int line) {
		if (err) {
			std::cerr << "\e[31mError at " << file << ':' << line << ": " << gluErrorString(err) << "\e[39m\n";
		}
	}

	void VBO::update(const void *data, GLsizeiptr size, bool sub, GLenum usage) {
		if (bind()) {
			if (sub) {
				glBufferSubData(GL_ARRAY_BUFFER, 0, size, data); CHECKGL_SET
				if (gl_err) {
					GLint64 d = -64;
					glGetBufferParameteri64v(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &d);
					INFO("size[" << size << "], d[" << d << "]");
				}
			} else {
				glBufferData(GL_ARRAY_BUFFER, size, data, usage); CHECKGL
			}
		}
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
}
