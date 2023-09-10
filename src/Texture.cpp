// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/texture.cpp
#include "Log.h"
#include "Texture.h"
#include "util/GL.h"
#include "util/Util.h"

#include <csignal>
#include <unordered_map>

// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <nlohmann/json.hpp>

namespace Game3 {
	static constexpr GLint DEFAULT_FILTER = GL_NEAREST;

	Texture::Texture():
		NamedRegisterable(Identifier()) {}

	Texture::Texture(Identifier identifier_, std::filesystem::path path_, bool alpha_, int filter_):
		NamedRegisterable(std::move(identifier_)),
		format(alpha_? GL_RGBA : GL_RGB),
		filter(filter_ == -1? DEFAULT_FILTER : filter_),
		alpha(alpha_),
		path(std::move(path_)) {}

	void Texture::init() {
		if (!valid) {
			int channels = 0;
			uint8_t *raw = stbi_load(path.c_str(), &width, &height, &channels, 0);
			if (raw == nullptr)
				throw std::runtime_error("Couldn't load image from " + path.string());
			init(std::shared_ptr<uint8_t[]>(raw, free));
		}
	}

	void Texture::init(std::shared_ptr<uint8_t[]> new_data) {
		if (!valid) {
			data = std::move(new_data);
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data.get());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
			glBindTexture(GL_TEXTURE_2D, 0);
			valid = true;
		}
	}

	void Texture::bind(int bind_id) {
		init();
		if (0 <= bind_id) {
			glActiveTexture(GL_TEXTURE0 + bind_id); CHECKGL
		}
		glBindTexture(GL_TEXTURE_2D, id); CHECKGL
	}

	static std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;

	std::shared_ptr<Texture> cacheTexture(const std::filesystem::path &path, bool alpha, int filter) {
		auto canonical = std::filesystem::canonical(path).string();
		if (textureCache.contains(canonical))
			return textureCache.at(canonical);
		return textureCache.try_emplace(canonical, std::make_shared<Texture>(Identifier(), path, alpha, filter == -1? DEFAULT_FILTER : filter)).first->second;
	}

	std::shared_ptr<Texture> cacheTexture(const char *path, bool alpha, int filter) {
		return cacheTexture(std::filesystem::path(path), alpha, filter == -1? DEFAULT_FILTER : filter);
	}

	std::shared_ptr<Texture> cacheTexture(const nlohmann::json &json) {
		const std::string path = json.at(0);
		if (auto iter = textureCache.find(path); iter != textureCache.end())
			return iter->second;
		return textureCache.try_emplace(path, std::make_shared<Texture>(json.get<Texture>())).first->second;
	}

	std::string Texture::filterToString(int filter) {
		filter = filter == -1? DEFAULT_FILTER : filter;
		switch (filter) {
			case GL_NEAREST: return "nearest";
			case GL_LINEAR:  return "linear";
			default: throw std::runtime_error("Unrecognized filter: " + std::to_string(filter));
		}
	}

	int Texture::stringToFilter(const std::string &string) {
		if (string == "nearest")
			return GL_NEAREST;

		if (string == "linear")
			return GL_LINEAR;

		throw std::runtime_error("Unrecognized filter: " + string);
	}

	void to_json(nlohmann::json &json, const Texture &texture) {
		json[0] = texture.path;
		json[1] = texture.alpha;
		json[2] = Texture::filterToString(texture.filter);
	}

	void from_json(const nlohmann::json &json, Texture &texture) {
		bool alpha = 1 < json.size()? json.at(1).get<bool>() : true;
		int filter = 2 < json.size()? Texture::stringToFilter(json.at(2)) : GL_NEAREST;
		texture = *cacheTexture(json.at(0), alpha, filter);
	}
}
