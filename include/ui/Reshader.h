#pragma once

#include <Eigen/Eigen>

#include "Shader.h"

namespace Game3 {

	/** For drawing a texture with a fragment shader applied. */
	class Reshader {
		public:
			Shader shader;

			Reshader() = delete;
			Reshader(std::string_view fragment_shader);
			~Reshader();

			void update(int backbuffer_width, int backbuffer_height);

			void operator()(GLuint texture);

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
