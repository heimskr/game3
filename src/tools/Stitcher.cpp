#include "config.h"
#include "Log.h"
#include "graphics/GL.h"
#include "graphics/Texture.h"
#include "tools/Stitcher.h"
#include "util/Crypto.h"
#include "util/FS.h"
#include "util/Util.h"

#include <cmath>

#include <nlohmann/json.hpp>

#ifdef USING_VCPKG
#include <stb_image.h>
#include <stb_image_write.h>
#else
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#endif

namespace Game3 {
	Tileset stitcher(const std::filesystem::path &base_dir, Identifier tileset_name, std::string *png_out) {
		std::set<std::filesystem::path> dirs;

		for (const std::filesystem::directory_entry &entry: std::filesystem::directory_iterator(base_dir))
			dirs.insert(entry);

		std::multimap<Identifier, std::string> autotiles;
		std::unordered_map<std::string, nlohmann::json> json_map;
		std::set<std::string> non_autotiles;
		std::unordered_map<std::string, std::unique_ptr<uint8_t[], FreeDeleter>> images;

		Tileset out(tileset_name);
		constexpr size_t tilesize = 16;
		out.tileSize = tilesize;
		out.empty = "base:tile/empty";
		out.missing = "base:tile/missing";
		Hasher hasher(Hasher::Algorithm::SHA3_512);

		for (const std::filesystem::path &dir: dirs) {
			nlohmann::json json = nlohmann::json::parse(readFile(dir / "tile.json"));
			std::filesystem::path png_path = dir / "tile.png";
			std::string name = dir.filename();

			int width{}, height{}, channels{};
			images.emplace(name, stbi_load(png_path.c_str(), &width, &height, &channels, 4));

			int desired_dimension = 16;
			if (auto autotile = json.find("autotile"); autotile != json.end()) {
				autotiles.emplace(*autotile, name);
				desired_dimension = 64;
			} else
				non_autotiles.insert(name);

			Identifier tilename = json.at("tilename");

			if (json.at("solid").get<bool>())
				out.solid.insert(tilename);

			if (json.at("land").get<bool>())
				out.land.insert(tilename);

			out.inverseCategories[tilename] = {};

			if (auto categories = json.find("categories"); categories != json.end()) {
				for (const nlohmann::json &category_json: *categories) {
					Identifier category{category_json};
					out.categories[category].insert(tilename);
					out.inverseCategories[tilename].insert(std::move(category));
				}
			}

			json_map[name] = std::move(json);

			if (width != desired_dimension)
				throw std::runtime_error("Invalid width for " + name + ": " + std::to_string(width) + " (expected " + std::to_string(desired_dimension) + ')');

			if (height != desired_dimension)
				throw std::runtime_error("Invalid height for " + name + ": " + std::to_string(height) + " (expected " + std::to_string(desired_dimension) + ')');

			if (channels != 3 && channels != 4)
				throw std::runtime_error("Invalid channel count for " + name + ": " + std::to_string(channels) + " (expected 3 or 4)");
		}

		// We want to represent the 4x4 autotile sets as 16 wide, 1 tall lines of tiles.
		// We then want to distribute those lines into a square whose dimension is a power of two,
		// but no larger than necessary to store all the lines plus all the remaining single tiles.
		const size_t single_tile_count = json_map.size() - autotiles.size();
		// For the sake of calculation, we can pretend that the tiles not part of autotile sets are
		// arranged into lines just like the autotile sets. We add one because the top left tile of
		// the tileset has to be empty.
		const size_t effective_autotile_sets = 1 + autotiles.size() + updiv(single_tile_count, 16);
		// If the effective number of lines is no more than 16, then the square will be 16x16 tiles.
		size_t dimension = 16;
		// Otherwise, we need to take the square root of the effective number of lines and round it
		// up to the nearest power of two. This will get us the final side length of the omnisquare
		// in tiles.
		if (16 < effective_autotile_sets)
			dimension = size_t(std::pow(2, std::ceil(std::log2(std::ceil(std::sqrt(16 * effective_autotile_sets))))));
		// Each tile is 16 pixels by 16 pixels.
		dimension *= tilesize;

		const size_t raw_byte_count = dimension * dimension * 4;
		auto raw = std::make_unique<uint8_t[]>(raw_byte_count); // 4 channels: RGBA

		// In pixels.
		size_t x_index = 256;
		size_t y_index = 0;

		size_t tile_index = 16;

		if (dimension == 16) {
			x_index = 0;
			y_index = 16;
		}

		const auto next = [&](size_t x_increment) {
			x_index += x_increment;
			tile_index += x_increment / tilesize;
			assert(x_index <= dimension);
			if (x_index == dimension) {
				x_index = 0;
				y_index += tilesize;
			}
		};

		for (const auto &[identifier, name]: autotiles) {
			hasher += json_map.at(name).dump();

			const auto &source = images.at(name);
			for (size_t row = 0; row < 4; ++row) {
				for (size_t column = 0; column < 4; ++column) {
					for (size_t y = 0; y < tilesize; ++y) {
						for (size_t x = 0; x < tilesize; ++x) {
							const size_t source_x = tilesize * column + x;
							const size_t source_y = tilesize * row + y;
							const size_t destination_x = (4 * row + column) * tilesize + x + x_index;
							const size_t destination_y = y + y_index;
							std::memcpy(&raw[4 * (destination_x + destination_y * dimension)], &source[4 * (source_x + 4 * tilesize * source_y)], 4 * sizeof(uint8_t));
						}
					}
				}
			}

			Identifier tilename = json_map.at(name).at("tilename");
			out.ids[tilename] = tile_index;
			out.names[tile_index] = std::move(tilename);

			next(16 * tilesize);
		}

		for (const auto &name: non_autotiles) {
			hasher += json_map.at(name).dump();

			const auto &source = images.at(name);
			for (size_t y = 0; y < tilesize; ++y)
				for (size_t x = 0; x < tilesize; ++x)
					std::memcpy(&raw[4 * (x_index + x + dimension * (y_index + y))], &source[4 * (tilesize * y + x)], 4 * sizeof(uint8_t));

			Identifier tilename = json_map.at(name).at("tilename");
			out.ids[tilename] = tile_index;
			out.names[tile_index] = std::move(tilename);

			next(tilesize);
		}

		out.hash = hasher.value<std::string>();
		INFO("Hash: " << hexString(out.hash, false));
		out.ids["base:tile/empty"] = 0;
		out.names[0] = "base:tile/empty";

		if (png_out != nullptr) {
			for (const auto &[key, name]: autotiles)
				INFO(name << " → " << out.ids[json_map.at(name)["tilename"]]);

			for (const auto &name: non_autotiles)
				INFO(name << " → " << out.ids[json_map.at(name)["tilename"]]);

			std::stringstream ss;

			stbi_write_png_to_func(+[](void *context, void *data, int size) {
				std::stringstream &ss = *reinterpret_cast<std::stringstream *>(context);
				ss << std::string_view(reinterpret_cast<const char *>(data), size);
			}, &ss, dimension, dimension, 4, raw.get(), dimension * 4);

			*png_out = ss.str();
		}


		auto texture = std::make_shared<Texture>(std::move(tileset_name));
		texture->alpha = true;
		texture->filter = GL_NEAREST;
		texture->format = GL_RGBA;
		texture->width = dimension;
		texture->height = dimension;
		texture->init(std::move(raw));

		out.cachedTexture = std::move(texture);

		return out;
	}
}
