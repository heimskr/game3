#pragma once


#include "graphics/Shader.h"
#include "graphics/GL.h"

#include <string_view>

#include "lib/Eigen.h"

namespace Game3 {
	class Texture;

	class Multiplier {
		public:
			Shader shader;

			Multiplier();
			~Multiplier();

			void reset();
			void update(int backbuffer_width, int backbuffer_height);
			void bind();

			void operator()(GLuint, GLuint, float width, float height);
			void operator()(const GL::Texture &, const GL::Texture &);
			void operator()(const Texture &, const Texture &);

			template <typename... Args>
			void set(const char *uniform, Args &&...args) {
				shader.set(uniform, std::forward<Args>(args)...);
			}

		private:
			void initRenderData();
			GLuint vbo = 0;
			GLuint quadVAO = 0;
			glm::mat4 projection;
			bool initialized = false;
			int backbufferWidth = -1;
			int backbufferHeight = -1;
	};
}
