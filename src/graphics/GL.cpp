#include "Log.h"
#include "graphics/GL.h"

namespace GL {
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
