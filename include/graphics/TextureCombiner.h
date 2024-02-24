#pragma once


#include "graphics/Shader.h"
#include "graphics/GL.h"

#include <string_view>

#include "lib/Eigen.h"

namespace Game3 {
	class Texture;

	class TextureCombiner {
		public:
			Shader shader;

			~TextureCombiner();

			void reset();
			void update(int backbuffer_width, int backbuffer_height);
			void bind();

			void operator()(GLuint, GLuint);
			void operator()(const GL::Texture &, const GL::Texture &);
			void operator()(const Texture &, const Texture &);

			template <typename... Args>
			void set(const char *uniform, Args &&...args) {
				shader.set(uniform, std::forward<Args>(args)...);
			}

		protected:
			TextureCombiner(const std::string &name, const std::string &vertex, const std::string &fragment);

		private:
			void initRenderData();
			GLuint vbo = 0;
			GLuint quadVAO = 0;
			glm::dmat4 projection;
			bool initialized = false;
			int backbufferWidth = -1;
			int backbufferHeight = -1;
	};
}
