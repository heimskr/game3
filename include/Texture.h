#pragma once

#include <filesystem>

#include <nlohmann/json.hpp>

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace Game3 {
	class Texture {
		public:
			std::shared_ptr<GLuint> id = std::make_shared<GLuint>(0);
			int width = 0;
			int height = 0;
			int format = 0;
			int filter = 0;
			bool alpha = false;
			std::filesystem::path path;
			std::shared_ptr<uint8_t> data;

			Texture() = default;
			Texture(const std::filesystem::path &, bool alpha_ = true, int filter_ = GL_NEAREST);

			void init();
			void bind();
			bool valid() const { return valid_; }

		private:
			bool valid_ = false;
	};

	Texture & cacheTexture(const std::filesystem::path &, bool alpha = true, int filter = GL_NEAREST);

	void to_json(nlohmann::json &, const Texture &);
	void from_json(const nlohmann::json &, Texture &);
}
