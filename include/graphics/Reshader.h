#pragma once

#include "graphics/Shader.h"
#include "graphics/GL.h"

#include "lib/Eigen.h"

#include <functional>
#include <string_view>

namespace Game3 {
	class Texture;
	class Tileset;
	class Window;
	struct RenderOptions;

	/** For drawing a texture with a fragment shader applied. */
	class Reshader {
		public:
			Shader shader;
			std::function<void(Shader &, GLint texture)> shaderSetup;

			Reshader() = delete;
			Reshader(std::string_view fragment_shader);
			~Reshader();

			void reset();
			void update(int backbuffer_width, int backbuffer_height);
			void bind();

			bool drawOnScreen(GLuint texture);
			bool drawOnScreen(const std::shared_ptr<Texture> &);
			bool drawOnScreen(const GL::Texture &);

			bool drawOnMap(const std::shared_ptr<Texture> &, const RenderOptions &, const Tileset &, const Window &);
			bool drawOnMap(const GL::Texture &, const RenderOptions &, const Tileset &, const Window &);

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

			bool draw(GLuint texture, const glm::mat4 &model);
			bool drawOnMap(GLuint texture, int texture_width, int texture_height, const RenderOptions &, const Tileset &, const Window &);
	};
}
