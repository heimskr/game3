// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/texture.cpp
#include "config.h"
#include "util/Log.h"
#include "graphics/GL.h"
#include "graphics/Texture.h"
#include "lib/JSON.h"
#include "lib/PNG.h"
#include "util/Util.h"

#include <unordered_map>

namespace Game3 {
	static constexpr GLint DEFAULT_FILTER = GL_NEAREST;

	static void setParameters(GLint filter) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter); CHECKGL
	}

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

		png::image<png::rgba_pixel> image(path.string().c_str());
		width = static_cast<int>(image.get_width());
		height = static_cast<int>(image.get_height());
		alpha = true;
		format = GL_RGBA;
		init(getRaw(image), width, height);
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
		setParameters(filter);
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
		setParameters(filter);
		glBindTexture(GL_TEXTURE_2D, 0); CHECKGL
		valid = true;
	}

	void Texture::init(std::span<const uint8_t> span, int data_width, int data_height) {
		width = data_width;
		height = data_height;
		glGenTextures(1, &id); CHECKGL
		assert(id != 0);
		glBindTexture(GL_TEXTURE_2D, id); CHECKGL
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, span.data()); CHECKGL
		setParameters(filter);
		glBindTexture(GL_TEXTURE_2D, 0); CHECKGL
		valid = true;
	}

	void Texture::repeat() {
		if (id == 0) {
			return;
		}

		glBindTexture(GL_TEXTURE_2D, id); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); CHECKGL
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); CHECKGL
		glBindTexture(GL_TEXTURE_2D, 0); CHECKGL
	}

	void Texture::init(const GL::Texture &from) {
		if (valid) {
			return;
		}

		width = from.getWidth();
		height = from.getHeight();
		id = from.getHandle();

		valid = true;
	}

	bool Texture::upload(std::span<const uint8_t> pixels) {
		if (!valid) {
			return false;
		}

		const std::size_t expected_byte_count = width * height * (alpha? 4 : 3);

		if (pixels.size() != expected_byte_count) {
			ERR("width = {}, height = {}", width, height);
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
		stbi_write_png(dump_path.string().c_str(), width, height, channels, data.get(), width * channels);
	}

	void Texture::destroy() {
		if (!valid) {
			return;
		}

		glDeleteTextures(1, &id); CHECKGL
		id = 0;
		valid = false;
	}

	using TextureCache = std::unordered_map<std::string, std::shared_ptr<Texture>>;

	static TextureCache & getTextureCache() {
		static TextureCache cache;
		return cache;
	}

	std::shared_ptr<Texture> cacheTexture(const std::filesystem::path &path, bool alpha, int filter) {
		std::string canonical = std::filesystem::canonical(path).string();
		TextureCache &cache = getTextureCache();
		if (auto iter = cache.find(canonical); iter != cache.end()) {
			return iter->second;
		}
		return cache.try_emplace(std::move(canonical), std::make_shared<Texture>(Identifier(), path, alpha, filter == -1? DEFAULT_FILTER : filter)).first->second;
	}

	std::shared_ptr<Texture> cacheTexture(const char *path, bool alpha, int filter) {
		return cacheTexture(std::filesystem::path(path), alpha, filter);
	}

	std::shared_ptr<Texture> cacheTexture(const boost::json::value &json) {
		std::string path(json.at(0).as_string());
		TextureCache &cache = getTextureCache();
		if (auto iter = cache.find(path); iter != cache.end()) {
			return iter->second;
		}
		return cache.try_emplace(std::move(path), boost::json::value_to<TexturePtr>(json)).first->second;
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

	int Texture::stringToFilter(std::string_view string) {
		if (string == "nearest") {
			return GL_NEAREST;
		}

		if (string == "linear") {
			return GL_LINEAR;
		}

		throw std::runtime_error(std::format("Unrecognized filter: {}", string));
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const TexturePtr &texture) {
		auto &array = json.emplace_array();
		array.emplace_back(boost::json::value_from(texture->path));
		array.emplace_back(texture->alpha);
		array.emplace_back(Texture::filterToString(texture->filter));
	}

	TexturePtr tag_invoke(boost::json::value_to_tag<TexturePtr>, const boost::json::value &json) {
		const auto &array = json.as_array();
		bool alpha = 1 < array.size()? array.at(1).as_bool() : true;
		int filter = 2 < array.size()? Texture::stringToFilter(std::string_view(array.at(2).as_string())) : GL_NEAREST;
		return cacheTexture(std::filesystem::path(std::string_view(json.at(0).as_string())), alpha, filter);
	}
}
