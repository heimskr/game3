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
			std::shared_ptr<GLuint> id  = std::make_shared<GLuint>(0);
			std::shared_ptr<int> width  = std::make_shared<int>(0);
			std::shared_ptr<int> height = std::make_shared<int>(0);
			std::shared_ptr<int> format = std::make_shared<int>(0);
			std::shared_ptr<int> filter = std::make_shared<int>(0);
			std::shared_ptr<bool> alpha = std::make_shared<bool>(false);
			std::filesystem::path path;
			std::shared_ptr<std::shared_ptr<uint8_t>> data = std::make_shared<std::shared_ptr<uint8_t>>();

			Texture() = default;
			Texture(const std::filesystem::path &, bool alpha_ = true, int filter_ = GL_NEAREST);

			void init();
			void bind();
			bool valid() const { return *valid_; }

		private:
			std::shared_ptr<bool> valid_ = std::make_shared<bool>(false);
	};

	Texture & cacheTexture(const std::filesystem::path &, bool alpha = true, int filter = GL_NEAREST);

	void to_json(nlohmann::json &, const Texture &);
	void from_json(const nlohmann::json &, Texture &);
}
