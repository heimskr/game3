#pragma once

#include "graphics/GL.h"
#include "lib/Eigen.h"

#include <string>

#include <glm/glm.hpp>

namespace Game3 {
	struct Color;

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
			Shader & set(const char *, GLfloat, GLfloat, GLfloat, GLfloat);
			Shader & set(const char *, GLdouble);
			Shader & set(const char *, GLdouble, GLdouble);
			Shader & set(const char *, GLdouble, GLdouble, GLdouble, GLdouble);
			Shader & set(const char *, const GLint *, GLsizei);
			Shader & set(const char *, const glm::mat4 &);
			Shader & set(const char *, const glm::dmat4 &);
			Shader & set(const char *, const Eigen::Vector2f &);
			Shader & set(const char *, const Eigen::Vector2d &);
			Shader & set(const char *, const Eigen::Vector4f &);
			Shader & set(const char *, const Eigen::Vector4d &);
			Shader & set(const char *, const Color &);

			template <template <typename...> typename C>
			Shader & set(const char *uniform_name, const C<GLint> &data) {
				return set(uniform_name, data.data(), data.size());
			}

			inline auto getHandle() const { return handle; }

			inline const auto & getName() const { return name; }

		private:
			std::string name;
			GLuint handle = 0;
	};
}
