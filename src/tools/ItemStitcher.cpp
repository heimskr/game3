#include "config.h"
#include "Log.h"
#include "game/Resource.h"
#include "graphics/GL.h"
#include "graphics/ItemSet.h"
#include "graphics/Texture.h"
#include "registry/Registries.h"
#include "threading/ThreadContext.h"
#include "tools/ItemStitcher.h"
#include "util/Crypto.h"
#include "util/FS.h"
#include "util/Util.h"

#include <cmath>

#include <nlohmann/json.hpp>

#ifdef USING_VCPKG
#include "lib/stb/stb_image.h"
#include "lib/stb/stb_image_write.h"
#else
#include "lib/stb/stb_image.h"
#include "lib/stb/stb_image_write.h"
#endif

namespace Game3 {
	ItemSet itemStitcher(ItemTextureRegistry *texture_registry, ResourceRegistry *resource_registry, const std::filesystem::path &base_dir, Identifier itemset_name, std::string *png_out) {
		std::set<std::filesystem::path> dirs;

		for (const std::filesystem::directory_entry &entry: std::filesystem::directory_iterator(base_dir))
			if (std::filesystem::is_directory(entry))
				dirs.insert(entry);

		std::unordered_map<std::string, nlohmann::json> jsons;
		std::unordered_map<std::string, std::unique_ptr<uint8_t[], FreeDeleter>> images;

		TexturePtr texture = std::make_shared<Texture>(itemset_name);
		texture->alpha  = true;
		texture->filter = GL_NEAREST;
		texture->format = GL_RGBA;

		ItemSet out(itemset_name);
		Hasher hasher(Hasher::Algorithm::SHA3_512);

		constexpr static size_t base_size = 16;

		size_t count_1x1 = 0;
		size_t count_2x2 = 0;

		std::set<std::string> names_1x1;
		std::set<std::string> names_2x2;

		if (std::filesystem::exists(base_dir / "itemset.json")) {
			const nlohmann::json itemset_meta = nlohmann::json::parse(readFile(base_dir / "itemset.json"));

			if (auto iter = itemset_meta.find("name"); iter != itemset_meta.end())
				out.name = *iter;
		}

		for (const std::filesystem::path &dir: dirs) {
			std::string name = dir.filename();
			std::filesystem::path png_path = base_dir / name / "item.png";
			jsons[name] = nlohmann::json::parse(readFile(dir / "item.json"));

			int width{}, height{}, channels{};
			images.emplace(name, stbi_load(png_path.c_str(), &width, &height, &channels, 4));

			if (channels != 3 && channels != 4)
				throw std::runtime_error(std::format("Invalid channel count for {} at {}: {} (expected 3 or 4)", name, png_path.c_str(), channels));

			if (width == 16 && height == 16) {
				++count_1x1;
				names_1x1.insert(std::move(name));
			} else if (width == 32 && height == 32) {
				++count_2x2;
				names_2x2.insert(std::move(name));
			} else
				throw std::runtime_error(std::format("Invalid dimensions for item {}: {}x{} (expected 16x16 or 32x32)", name, width, height));
		}

		const size_t effective_2x2s = count_2x2 + updiv(count_1x1, 4);
		size_t dimension = base_size * static_cast<size_t>(std::pow(2, std::ceil(std::log2(std::ceil(std::sqrt(4 * effective_2x2s))))));

		const size_t raw_byte_count = dimension * dimension * 4;
		auto raw = std::make_unique<uint8_t[]>(raw_byte_count); // 4 channels: RGBA

		// In pixels.
		size_t x_index = 0;
		size_t y_index = 0;

		const size_t remainder = count_2x2 % (dimension / base_size / 2);
		const size_t last_row_padding = 2 * base_size * remainder;
		int rows_left = 2;

		const auto next = [&](size_t pixel_increment) {
			x_index += pixel_increment;
			assert(x_index <= dimension);
			if (x_index == dimension) {
				if (pixel_increment == 16) {
					x_index = --rows_left == 1? last_row_padding : 0;
					y_index += base_size;
				} else {
					x_index = 0;
					y_index += 2 * base_size;
				}
			}
		};

		auto handle_json = [&](const std::string &name, int scale) {
			if (auto iter = jsons.find(name); iter != jsons.end()) {
				const nlohmann::json &json = iter->second;
				hasher += json.dump();
				Identifier id = json.at("id");

				if (texture_registry)
					texture_registry->add(id, ItemTexture{id, texture, int(x_index), int(y_index), int(scale * base_size), int(scale * base_size)});

				if (resource_registry)
					if (auto iter = json.find("resource"); iter != json.end())
						resource_registry->add(id, Resource{id, *iter});
			}
		};

		for (const std::string &name: names_2x2) {
			hasher += name;

			const std::unique_ptr<uint8_t[], FreeDeleter> &source = images.at(name);

			for (size_t y = 0; y < base_size * 2; ++y) {
				for (size_t x = 0; x < base_size * 2; ++x) {
					const size_t source_index = 4 * (x + y * base_size * 2);
					const size_t raw_index = 4 * (x_index + x + (y_index + y) * dimension);
					std::memcpy(&raw[raw_index], &source[source_index], 4);
				}
			}

			handle_json(name, 2);
			next(2 * base_size);
		}

		for (const std::string &name: names_1x1) {
			hasher += name;

			const std::unique_ptr<uint8_t[], FreeDeleter> &source = images.at(name);

			for (size_t y = 0; y < base_size; ++y) {
				for (size_t x = 0; x < base_size; ++x) {
					const size_t source_index = 4 * (x + y * base_size);
					const size_t raw_index = 4 * (x_index + x + (y_index + y) * dimension);
					std::memcpy(&raw[raw_index], &source[source_index], 4);
				}
			}

			handle_json(name, 1);
			next(base_size);
		}

		out.hash = hexString(hasher.value<std::string>(), false);

		if (png_out != nullptr) {
			std::stringstream ss;

			stbi_write_png_to_func(+[](void *context, void *data, int size) {
				std::stringstream &ss = *reinterpret_cast<std::stringstream *>(context);
				ss << std::string_view(reinterpret_cast<const char *>(data), size);
			}, &ss, dimension, dimension, 4, raw.get(), dimension * 4);

			*png_out = ss.str();
		}

		texture->width = dimension;
		texture->height = dimension;
		texture->init(std::move(raw));

		out.cachedTexture = std::move(texture);

		return out;
	}
}
