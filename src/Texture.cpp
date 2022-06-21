// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/texture.cpp
#include <csignal>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Texture.h"
#include "util/Util.h"

namespace Game3 {
	Texture::Texture(const std::filesystem::path &path_, bool alpha_, int filter_):
		format(std::make_shared<int>(alpha_? GL_RGBA : GL_RGB)), filter(std::make_shared<int>(filter_)), alpha(std::make_shared<bool>(alpha_)), path(path_) {}

	void Texture::init() {
		if (!*valid_) {
			int channels = 0;
			uint8_t *raw = stbi_load(path.c_str(), width.get(), height.get(), &channels, 0);
			if (raw == nullptr)
				throw std::runtime_error("Couldn't load image from " + path.string());
			glGenTextures(1, id.get());
			glBindTexture(GL_TEXTURE_2D, *id);
			glTexImage2D(GL_TEXTURE_2D, 0, *format, *width, *height, 0, *format, GL_UNSIGNED_BYTE, raw);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, *filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, *filter);
			glBindTexture(GL_TEXTURE_2D, 0);
			*valid_ = true;
			*data = std::shared_ptr<uint8_t>(raw);
		}
	}

	void Texture::bind() {
		init();
		glBindTexture(GL_TEXTURE_2D, *id); CHECKGL
	}


	Texture & cacheTexture(const std::filesystem::path &path, bool alpha, int filter) {
		static std::unordered_map<std::string, Texture> textureCache;
		auto canonical = std::filesystem::canonical(path).string();
		if (textureCache.contains(canonical))
			return textureCache.at(canonical);
		return textureCache.try_emplace(canonical, canonical, alpha, filter).first->second;
	}

	void to_json(nlohmann::json &json, const Texture &texture) {
		json["alpha"]  = *texture.alpha;
		json["filter"] = *texture.filter;
		json["path"]   = texture.path;
	}

	void from_json(const nlohmann::json &json, Texture &texture) {
		texture = cacheTexture(json.at("path"), json.at("alpha"), json.at("filter"));
	}
}
