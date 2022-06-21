#pragma once

#include <string>

#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#include <Eigen/Eigen>
#include <glm/glm.hpp>

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace Game3 {
	class Shader {
		public:
			Shader() = delete;
			Shader(const std::string &name_): name(name_) {}
			Shader(std::string &&name_): name(std::move(name_)) {}

			~Shader();

			Shader & operator=(Shader &&);

			void init(const std::string &vertex, const std::string &fragment, const std::string &geometry = {});
			void bind();
			GLint uniform(const char *, bool warn = true) const;
			GLint uniform(const std::string &, bool warn = true) const;
			void reset();

			Shader & set(const char *, const glm::mat4 &);
			Shader & set(const char *, const Eigen::Vector4f &);
			Shader & set(const char *, float x, float y, float z, float w);

			GLuint getHandle() const { return handle; }

		private:
			std::string name;
			GLuint handle = 0;
	};
}
