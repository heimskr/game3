#include "Log.h"
#include "tools/Stitcher.h"
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

#include <malloc.h>

namespace Game3 {
	Tileset stitcher(const std::filesystem::path &base_dir, Identifier tileset_name) {
		std::set<std::filesystem::path> dirs;

		for (const std::filesystem::directory_entry &entry: std::filesystem::directory_iterator(base_dir))
			dirs.insert(entry);

		std::multimap<Identifier, std::string> autotiles;
		std::unordered_map<std::string, nlohmann::json> json_map;
		std::set<std::string> non_autotiles;
		std::unordered_map<std::string, std::unique_ptr<uint8_t[], FreeDeleter>> images;

		Tileset out(std::move(tileset_name));

		for (const std::filesystem::path &dir: dirs) {
			nlohmann::json json = nlohmann::json::parse(readFile(dir / "tile.json"));
			std::string name = dir.filename();
			std::filesystem::path png_path = dir / "tile.png";
			int width{}, height{}, channels{};
			images.emplace(name, stbi_load(png_path.c_str(), &width, &height, &channels, 4));
			int desired_dimension = 16;
			if (auto autotile = json.find("autotile"); autotile != json.end()) {
				autotiles.emplace(*autotile, name);
				desired_dimension = 64;
			} else
				non_autotiles.insert(name);

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
		// arranged into lines just like the autotile sets.
		const size_t effective_autotile_sets = autotiles.size() + updiv(single_tile_count, 16);
		// If the effective number of lines is no more than 16, then the square will be 16x16 tiles.
		size_t dimension = 16;
		// Otherwise, we need to take the square root of the effective number of lines and round it
		// up to the nearest power of two. This will get us the final side length of the omnisquare
		// in tiles.
		if (16 < effective_autotile_sets)
			dimension = size_t(std::pow(2, std::ceil(std::log2(std::ceil(std::sqrt(16 * effective_autotile_sets))))));
		// Each tile is 16 pixels by 16 pixels.
		dimension *= 16;

		auto raw = std::make_unique<uint8_t[]>(dimension * dimension * 4); // 4 channels: RGBA

		// In pixels.
		size_t x_index = 0;
		size_t y_index = 0;

		const auto next = [&](size_t x_increment) {
			x_index += x_increment;
			if (x_index == dimension) {
				x_index = 0;
				y_index += 16;
			}
		};

		for (const auto &[identifier, name]: autotiles) {
			const auto &source = images.at(name);
			for (size_t row = 0; row < 4; ++row) {
				for (size_t column = 0; column < 4; ++column) {
					for (size_t y = 0; y < 16; ++y) {
						for (size_t x = 0; x < 16; ++x) {
							const size_t source_x = 16 * column + x;
							const size_t source_y = 16 * row + y;
							const size_t destination_x = (4 * row + column) * 16 + x + x_index;
							const size_t destination_y = y + y_index;
							std::memcpy(&raw[4 * (destination_x + destination_y * dimension)], &source[4 * (source_x + 64 * source_y)], 4 * sizeof(uint8_t));
						}
					}
				}
			}

			next(256);
		}

		for (const auto &name: non_autotiles) {
			const auto &source = images.at(name);
			for (size_t y = 0; y < 16; ++y)
				for (size_t x = 0; x < 16; ++x)
					std::memcpy(&raw[4 * (x_index + x + dimension * (y_index + y))], &source[4 * (16 * y + x)], 4 * sizeof(uint8_t));
			next(16);
		}

		stbi_write_png_to_func(+[](void *, void *data, int size) {
			std::cout << std::string_view(reinterpret_cast<const char *>(data), size);
		}, nullptr, dimension, dimension, 4, raw.get(), dimension * 4);

		return out;
	}
}
