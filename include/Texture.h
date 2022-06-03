#pragma once

#include <filesystem>

#include <nlohmann/json.hpp>
#include <GL/gl.h>

namespace Game3 {
	class Texture {
		public:
			GLuint id = 0;
			int width = 0;
			int height = 0;
			int format = 0;
			int filter = 0;
			bool alpha = false;
			std::filesystem::path path;

			Texture() = default;
			Texture(const std::filesystem::path &, bool alpha_ = true, int filter_ = GL_NEAREST);

			void bind();
			bool valid() const { return valid_; }

		private:
			bool valid_ = false;
	};

	void to_json(nlohmann::json &, const Texture &);
}
