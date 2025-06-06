#include "graphics/Color.h"
#include "graphics/GL.h"
#include "graphics/Shader.h"
#include "math/Vector.h"
#include "types/Position.h"
#include "types/Types.h"
#include "util/Debug.h"
#include "util/Log.h"
#include "util/Util.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#ifdef __MINGW32__
const GLubyte * gluErrorString(GLenum) {
	return reinterpret_cast<const GLubyte *>("???");
}
#endif

namespace Game3 {
	namespace {
		void check(const std::string &name, int handle, bool is_link = false) {
			int success{};
			std::array<char, 2048> info{"No info available"};

			if (is_link) {
				glGetProgramiv(handle, GL_LINK_STATUS, &success); CHECKGL
			} else {
				glGetShaderiv(handle, GL_COMPILE_STATUS, &success); CHECKGL
			}

			if (!success) {
				GLsizei len{};

				if (is_link) {
					glGetProgramInfoLog(handle, GL_INFO_LOG_LENGTH, &len, info.data()); CHECKGL
				} else {
					glGetShaderInfoLog(handle, 2048, &len, info.data()); CHECKGL
				}

				if (is_link) {
					ERR("Shader.cpp: error with handle {} (name = \"{}\", linking): {}", handle, name, info.data());
				} else {
					ERR("Shader.cpp: error with handle {} (name = \"{}\"): {}", handle, name, info.data());
				}

				Break();
			}
		}
	}

	Shader::~Shader() {
		reset();
	}

	Shader & Shader::operator=(Shader &&other) noexcept {
		reset();
		name = std::move(other.name);
		handle = other.handle;
		other.handle = 0;
		return *this;
	}

	void Shader::init(std::string_view vertex, std::string_view fragment, std::string_view geometry) {
		CHECKGL

		if (handle != 0) {
			glDeleteProgram(handle); CHECKGL
		}

		const GLchar *vert_ptr = reinterpret_cast<const GLchar *>(vertex.begin());
		const GLuint vert_handle = glCreateShader(GL_VERTEX_SHADER); CHECKGL
		const GLint vertex_size = vertex.size();
		glShaderSource(vert_handle, 1, &vert_ptr, &vertex_size); CHECKGL
		glCompileShader(vert_handle); CHECKGL
		check(name, vert_handle); CHECKGL

		const GLchar *frag_ptr = reinterpret_cast<const GLchar *>(fragment.begin());
		const GLuint frag_handle = glCreateShader(GL_FRAGMENT_SHADER); CHECKGL
		const GLint frag_size = fragment.size();
		glShaderSource(frag_handle, 1, &frag_ptr, &frag_size); CHECKGL
		glCompileShader(frag_handle); CHECKGL
		check(name, frag_handle); CHECKGL

		GLuint geom_handle = 0;

		if (!geometry.empty()) {
			const GLchar *geom_ptr = reinterpret_cast<const GLchar *>(geometry.begin());
			geom_handle = glCreateShader(GL_GEOMETRY_SHADER); CHECKGL
			const GLint geom_size = geometry.size();
			glShaderSource(geom_handle, 1, &geom_ptr, &geom_size); CHECKGL
			glCompileShader(geom_handle); CHECKGL
			check(name, geom_handle); CHECKGL
		}

		handle = glCreateProgram(); CHECKGL
		glAttachShader(handle, vert_handle); CHECKGL
		glAttachShader(handle, frag_handle); CHECKGL
		if (geom_handle != 0) {
			glAttachShader(handle, geom_handle); CHECKGL
		}
		glLinkProgram(handle); CHECKGL
		check(name, handle, true); CHECKGL

		glDetachShader(handle, vert_handle); CHECKGL
		glDeleteShader(vert_handle); CHECKGL

		glDetachShader(handle, frag_handle); CHECKGL
		glDeleteShader(frag_handle); CHECKGL

		if (geom_handle != 0) {
			glDetachShader(handle, geom_handle); CHECKGL
			glDeleteShader(geom_handle); CHECKGL
		}
	}

	void Shader::bind() {
		CHECKGL
		if (handle == 0)
			throw std::runtime_error("Can't bind uninitialized shader");
		glUseProgram(handle); CHECKGL
	}

	GLint Shader::uniform(const char *uniform_name, bool warn) const {
		GLint id = glGetUniformLocation(handle, uniform_name); CHECKGL
		if (id == -1 && warn)
			std::cerr << "Couldn't find uniform \"" << uniform_name << "\" in shader \"" << name << "\"\n";
		return id;
	}

	GLint Shader::uniform(const std::string &uniform_name, bool warn) const {
		return uniform(uniform_name.c_str(), warn);
	}

	void Shader::reset() {
		if (handle != 0) {
			glDeleteProgram(handle); CHECKGL
			handle = 0;
		}
	}

	Shader & Shader::set(const char *uniform_name, GLint value) {
		glUniform1i(uniform(uniform_name), value);
#ifndef HIDE_CHECKGL
		if (auto err = glGetError()) { std::cerr << "\e[31mError at " << __FILE__ << ':' << (__LINE__-1) << ": " << gluErrorString(err) << "\e[39m, \"" << uniform_name << "\" -> " << value << '\n'; }
#endif
		return *this;
	}

	Shader & Shader::set(const char *uniform_name, GLfloat value) {
		glUniform1f(uniform(uniform_name), value);
#ifndef HIDE_CHECKGL
		if (auto err = glGetError()) { std::cerr << "\e[31mError at " << __FILE__ << ':' << (__LINE__-1) << ": " << gluErrorString(err) << "\e[39m, \"" << uniform_name << "\" -> " << value << '\n'; }
#endif
		return *this;
	}

	Shader & Shader::set(const char *uniform_name, GLfloat first, GLfloat second) {
		glUniform2f(uniform(uniform_name), first, second);
#ifndef HIDE_CHECKGL
		if (auto err = glGetError()) { std::cerr << "\e[31mError at " << __FILE__ << ':' << (__LINE__-1) << ": " << gluErrorString(err) << "\e[39m, \"" << uniform_name << "\" -> (" << first << ", " << second << ")\n"; }
#endif
		return *this;
	}

	Shader & Shader::set(const char *uniform_name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
		glUniform4f(uniform(uniform_name), x, y, z, w); CHECKGL
		return *this;
	}

	Shader & Shader::set(const char *uniform_name, const GLint *data, GLsizei count) {
		glUniform1iv(uniform(uniform_name), count, data); CHECKGL
		return *this;
	}

	Shader & Shader::set(const char *uniform_name, const glm::mat4 &matrix) {
		glUniformMatrix4fv(uniform(uniform_name), 1, GL_FALSE, glm::value_ptr(matrix)); CHECKGL
		return *this;
	}

	Shader & Shader::set(const char *uniform_name, const Vector2d &vector) {
		glUniform2f(uniform(uniform_name), static_cast<GLfloat>(vector.x), static_cast<GLfloat>(vector.y)); CHECKGL
		return *this;
	}

	Shader & Shader::set(const char *uniform_name, const Color &color) {
		glUniform4f(uniform(uniform_name), color.red, color.green, color.blue, color.alpha); CHECKGL
		return *this;
	}
}
