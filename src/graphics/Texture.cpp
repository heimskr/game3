// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/texture.cpp
#include "config.h"
#include "Log.h"
#include "graphics/GL.h"
#include "graphics/Texture.h"
#include "util/Util.h"

#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#ifdef USING_VCPKG
#include "lib/stb/stb_image.h"
#else
#include "lib/stb/stb_image.h"
#endif

#include <nlohmann/json.hpp>

namespace Game3 {
	static constexpr GLint DEFAULT_FILTER = GL_NEAREST;

	Texture::Texture(Identifier identifier, bool alpha, int filter):
		Texture(std::move(identifier), std::filesystem::path{}, alpha, filter) {}

	Texture::Texture(Identifier identifier, std::filesystem::path path, bool alpha, int filter):
		NamedRegisterable(std::move(identifier)),
		format(alpha? GL_RGBA : GL_RGB),
		filter(filter == -1? DEFAULT_FILTER : filter),
		alpha(alpha),
		path(std::move(path)) {}

	void Texture::init() {
		if (valid) {
			return;
		}

		int channels = 0;
		uint8_t *raw = stbi_load(path.c_str(), &width, &height, &channels, 0);
		alpha = channels == 4;
		format = alpha? GL_RGBA : GL_RGB;
		if (raw == nullptr) {
			throw std::runtime_error("Couldn't load image from " + path.string());
		}
		init(std::shared_ptr<uint8_t[]>(raw, stbi_image_free), width, height);
	}

	void Texture::init(int data_width, int data_height) {
		if (valid) {
			return;
		}

		data.reset();
		width = data_width;
		height = data_height;
		glGenTextures(1, &id); CHECKGL
		assert(id != 0);
		glBindTexture(GL_TEXTURE_2D, id); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter); CHECKGL
		glBindTexture(GL_TEXTURE_2D, 0); CHECKGL
		valid = true;
	}

	void Texture::init(std::shared_ptr<uint8_t[]> new_data, int data_width, int data_height) {
		if (valid) {
			return;
		}

		width = data_width;
		height = data_height;
		data = std::move(new_data);
		glGenTextures(1, &id); CHECKGL
		assert(id != 0);
		glBindTexture(GL_TEXTURE_2D, id); CHECKGL
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data.get()); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter); CHECKGL
		glBindTexture(GL_TEXTURE_2D, 0); CHECKGL
		valid = true;
	}

	bool Texture::upload(std::span<const uint8_t> pixels) {
		if (!valid) {
			return false;
		}

		const std::size_t expected_byte_count = width * height * (alpha? 4 : 3);

		if (pixels.size() != expected_byte_count) {
			ERROR("width = {}, height = {}", width, height);
			throw std::runtime_error(std::format("Expected {} pixel bytes, got {}", expected_byte_count, pixels.size()));
		}

		glBindTexture(GL_TEXTURE_2D, id); CHECKGL
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels.data()); CHECKGL
		glBindTexture(GL_TEXTURE_2D, 0); CHECKGL

		return true;
	}

	void Texture::bind(int bind_id) {
		init();
		if (0 <= bind_id) {
			glActiveTexture(GL_TEXTURE0 + bind_id); CHECKGL
		}
		glBindTexture(GL_TEXTURE_2D, id); CHECKGL
	}

	void Texture::dump(const std::filesystem::path &dump_path) {
		assert(valid);
		const int channels = alpha? 4 : 3;
		stbi_write_png(dump_path.c_str(), width, height, channels, data.get(), width * channels);
	}

	void Texture::destroy() {
		if (!valid) {
			return;
		}

		glDeleteTextures(1, &id); CHECKGL
		id = 0;
		valid = false;
	}

	static std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;

	std::shared_ptr<Texture> cacheTexture(const std::filesystem::path &path, bool alpha, int filter) {
		auto canonical = std::filesystem::canonical(path).string();
		if (textureCache.contains(canonical)) {
			return textureCache.at(canonical);
		}
		return textureCache.try_emplace(canonical, std::make_shared<Texture>(Identifier(), path, alpha, filter == -1? DEFAULT_FILTER : filter)).first->second;
	}

	std::shared_ptr<Texture> cacheTexture(const char *path, bool alpha, int filter) {
		return cacheTexture(std::filesystem::path(path), alpha, filter);
	}

	std::shared_ptr<Texture> cacheTexture(const nlohmann::json &json) {
		const std::string path = json.at(0);
		if (auto iter = textureCache.find(path); iter != textureCache.end()) {
			return iter->second;
		}
		return textureCache.try_emplace(path, json.get<TexturePtr>()).first->second;
	}

	std::string Texture::filterToString(int filter) {
		filter = filter == -1? DEFAULT_FILTER : filter;
		switch (filter) {
			case GL_NEAREST: return "nearest";
			case GL_LINEAR:  return "linear";
			default:
				throw std::runtime_error(std::format("Unrecognized filter: {}", filter));
		}
	}

	int Texture::stringToFilter(const std::string &string) {
		if (string == "nearest") {
			return GL_NEAREST;
		}

		if (string == "linear") {
			return GL_LINEAR;
		}

		throw std::runtime_error(std::format("Unrecognized filter: {}", string));
	}

	void to_json(nlohmann::json &json, const TexturePtr &texture) {
		json[0] = texture->path;
		json[1] = texture->alpha;
		json[2] = Texture::filterToString(texture->filter);
	}

	void from_json(const nlohmann::json &json, TexturePtr &texture) {
		bool alpha = 1 < json.size()? json.at(1).get<bool>() : true;
		int filter = 2 < json.size()? Texture::stringToFilter(json.at(2)) : GL_NEAREST;
		texture = cacheTexture(json.at(0), alpha, filter);
	}
}
