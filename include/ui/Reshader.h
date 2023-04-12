#pragma once

#include <Eigen/Eigen>

#include "Shader.h"
#include "util/GL.h"

namespace Game3 {
	/** For drawing a texture with a fragment shader applied. */
	class Reshader {
		public:
			Shader shader;

			Reshader() = delete;
			Reshader(std::string_view fragment_shader);
			~Reshader();

			void reset();
			void update(int backbuffer_width, int backbuffer_height);
			void bind();

			void operator()(GLuint texture);
			void operator()(const GL::Texture &);

			template <typename... Args>
			void set(const char *uniform, Args &&...args) {
				shader.set(uniform, std::forward<Args>(args)...);
			}

		private:
			void initRenderData();
			GLuint vbo = 0;
			GLuint quadVAO = 0;
			bool initialized = false;
			int backbufferWidth = -1;
			int backbufferHeight = -1;
	};
}
