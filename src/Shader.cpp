#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

namespace Game3 {
	Shader::~Shader() {
		reset();
	}

	Shader & Shader::operator=(Shader &&other) {
		reset();
		name = std::move(other.name);
		handle = other.handle;
		other.handle = 0;
		return *this;
	}

	static void check(int handle, bool is_link = false) {
		int success;
		char info[1024];
		if (is_link)
			glGetProgramiv(handle, GL_LINK_STATUS, &success);
		else
			glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLsizei len = 666;
			if (is_link)
				glGetProgramInfoLog(handle, GL_INFO_LOG_LENGTH, &len, info);
			else
				glGetShaderInfoLog(handle, 1024, &len, info);
			std::cerr << "Error with " << handle << " (l=" << len << "): " << info << '\n';
		}
	}

	void Shader::init(const std::string &vertex, const std::string &fragment, const std::string &geometry) {
		if (handle != 0)
			glDeleteProgram(handle);

		const GLchar *vert_ptr = reinterpret_cast<const GLchar *>(vertex.c_str());
		const GLuint vert_handle = glCreateShader(GL_VERTEX_SHADER);
		const GLint vertex_size = vertex.size();
		glShaderSource(vert_handle, 1, &vert_ptr, &vertex_size);
		glCompileShader(vert_handle);
		check(vert_handle);

		const GLchar *frag_ptr = reinterpret_cast<const GLchar *>(fragment.c_str());
		const GLuint frag_handle = glCreateShader(GL_FRAGMENT_SHADER);
		const GLint frag_size = fragment.size();
		glShaderSource(frag_handle, 1, &frag_ptr, &frag_size);
		glCompileShader(frag_handle);
		check(frag_handle);

		GLuint geom_handle = 0;

		if (!geometry.empty()) {
			const GLchar *geom_ptr = reinterpret_cast<const GLchar *>(geometry.c_str());
			geom_handle = glCreateShader(GL_GEOMETRY_SHADER);
			const GLint geom_size = geometry.size();
			glShaderSource(geom_handle, 1, &geom_ptr, &geom_size);
			glCompileShader(geom_handle);
			check(geom_handle);
		}

		handle = glCreateProgram();
		glAttachShader(handle, vert_handle);
		glAttachShader(handle, frag_handle);
		glAttachShader(handle, geom_handle);
		glLinkProgram(handle);
		check(handle, true);

		glDetachShader(handle, vert_handle);
		glDeleteShader(vert_handle);

		glDetachShader(handle, frag_handle);
		glDeleteShader(frag_handle);

		if (geom_handle != 0) {
			glDetachShader(handle, geom_handle);
			glDeleteShader(geom_handle);
		}
	}

	void Shader::bind() {
		if (handle == 0)
			throw std::runtime_error("Can't bind uninitialized shader");
		glUseProgram(handle);
	}

	GLint Shader::uniform(const char *uniform_name, bool warn) const {
		GLint id = glGetUniformLocation(handle, uniform_name);
		if (id == -1 && warn)
			std::cerr << "Couldn't find uniform \"" << uniform_name << "\" in shader \"" << name << "\"\n";
		return id;
	}

	GLint Shader::uniform(const std::string &uniform_name, bool warn) const {
		return uniform(uniform_name.c_str(), warn);
	}

	void Shader::reset() {
		if (handle != 0)
			glDeleteProgram(handle);
		handle = 0;
	}

	Shader & Shader::set(const char *uniform_name, const glm::mat4 &matrix) {
		glUniformMatrix4fv(uniform(uniform_name), 1, GL_FALSE, glm::value_ptr(matrix));
		return *this;
	}

	Shader & Shader::set(const char *uniform_name, const Eigen::Vector4f &vector) {
		glUniform4f(uniform(uniform_name), vector.x(), vector.y(), vector.z(), vector.w());
		return *this;
	}

	Shader & Shader::set(const char *uniform_name, float x, float y, float z, float w) {
		glUniform4f(uniform(uniform_name), x, y, z, w);
		return *this;
	}
}
