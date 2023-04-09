#pragma once

#include <string>

#include <Eigen/Eigen>
#include <glm/glm.hpp>
#include "util/GL.h"

namespace Game3 {
	class Shader {
		public:
			Shader() = delete;
			Shader(std::string name_): name(std::move(name_)) {}

			~Shader();

			Shader & operator=(Shader &&);

			void init(std::string_view vertex, std::string_view fragment, std::string_view geometry = {});
			void bind();
			GLint uniform(const char *, bool warn = true) const;
			GLint uniform(const std::string &, bool warn = true) const;
			void reset();

			Shader & set(const char *, GLint);
			Shader & set(const char *, GLfloat);
			Shader & set(const char *, GLfloat, GLfloat);
			Shader & set(const char *, const GLint *, GLsizei);
			Shader & set(const char *, const glm::mat4 &);
			Shader & set(const char *, const Eigen::Vector4f &);
			Shader & set(const char *, float x, float y, float z, float w);

			template <template <typename...> typename C>
			Shader & set(const char *uniform_name, const C<GLint> &data) {
				return set(uniform_name, data.data(), data.size());
			}

			GLuint getHandle() const { return handle; }

		private:
			std::string name;
			GLuint handle = 0;
	};
}
