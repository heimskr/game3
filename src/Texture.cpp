// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/texture.cpp
#include <iostream>
#include <stb_image.h>

#include "Texture.h"

namespace Game3 {
	Texture::Texture(const std::filesystem::path &path_, bool alpha_, int filter_):
	format(alpha_? GL_RGBA : GL_RGB), filter(filter_), alpha(alpha_), path(path_) {
		int channels = 0;
		unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (data == nullptr)
			throw std::runtime_error("Couldn't load image from " + path.string());
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		glBindTexture(GL_TEXTURE_2D, 0);
		valid_ = true;
	}

	void Texture::bind() {
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void to_json(nlohmann::json &json, const Texture &texture) {
		json["alpha"] = texture.alpha;
		json["filter"] = texture.filter;
		json["path"] = texture.path;
	}
}
