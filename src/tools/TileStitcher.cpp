#include "config.h"
#include "util/Log.h"
#include "graphics/GL.h"
#include "graphics/Texture.h"
#include "lib/JSON.h"
#include "tools/TileStitcher.h"
#include "util/Crypto.h"
#include "util/FS.h"
#include "util/Util.h"

#include <cmath>

#ifdef USING_VCPKG
#include "lib/stb/stb_image.h"
#include "lib/stb/stb_image_write.h"
#else
#include "lib/stb/stb_image.h"
#include "lib/stb/stb_image_write.h"
#endif

namespace Game3 {
	Tileset tileStitcher(const std::filesystem::path &base_dir, Identifier tileset_name, Side side, std::string *png_out) {
		std::set<std::filesystem::path> dirs;

		for (const std::filesystem::directory_entry &entry: std::filesystem::directory_iterator(base_dir)) {
			dirs.insert(entry);
		}

		std::unordered_map<std::string, JSON::value> json_map;
		std::set<std::string> short_autotiles;
		std::set<std::string> non_autotiles;
		std::set<std::string> tall_autotiles;
		std::unordered_map<std::string, std::unique_ptr<uint8_t[], FreeDeleter>> images;

		Tileset out(tileset_name);
		constexpr size_t tilesize = 16;
		out.tileSize = tilesize;
		out.empty = "base:tile/empty";
		out.missing = "base:tile/missing";
		Hasher hasher(Hasher::Algorithm::SHA3_512);

		for (const std::filesystem::path &dir: dirs) {
			if (!std::filesystem::is_directory(dir)) {
				continue;
			}

			JSON::value json = JSON::parse(readFile(dir / "tile.json"));
			std::string name = dir.filename().string();

			Identifier tilename = JSON::value_to<Identifier>(json.at("tilename"));

			out.inverseCategories[tilename] = {"base:category/all_tiles"};
			out.categories["base:category/all_tiles"].insert(tilename);

			if (auto *object = json.if_object()) {
				if (auto *categories = object->if_contains("categories")) {
					if (auto *categories_array = categories->if_array()) {
						for (const JSON::value &category_json: *categories_array) {
							Identifier category = JSON::value_to<Identifier>(category_json);
							out.categories[category].insert(tilename);
							out.inverseCategories[tilename].insert(std::move(category));
						}
					}
				}

				if (auto *stack = object->if_contains("stack")) {
					out.stackNames[tilename] = JSON::value_to<Identifier>(*stack);
				}
			}

			json_map[name] = std::move(json);
		}

		if (std::filesystem::exists(base_dir / "tileset.json")) {
			JSON::value tileset_meta = JSON::parse(readFile(base_dir / "tileset.json"));

			if (JSON::object *meta_object = tileset_meta.if_object()) {
				if (auto *autotiles = meta_object->if_contains("autotiles")) {
					for (const auto &[autotile, member]: autotiles->as_object()) {
						Identifier autotile_id{autotile};
						Identifier member_id = JSON::value_to<Identifier>(member);

						if (const std::string path_start = member_id.getPathStart(); path_start == "category") {
							for (const Identifier &tilename: out.getTilesByCategory(member_id)) {
								out.setAutotile(tilename, autotile_id);
							}
						} else if (path_start == "tile") {
							out.setAutotile(member_id, autotile_id);
						} else {
							throw std::runtime_error("Invalid autotile member: " + member_id.str());
						}
					}
				}

				// Some tiles, such as fences, want to autotile with most tiles.
				// The "omnitiles" member is a set of such autotiles.
				if (auto *omnitiles = meta_object->if_contains("omnitiles")) {
					for (const auto &omnitile: omnitiles->as_array()) {
						out.autotileSets.at(JSON::value_to<Identifier>(omnitile))->omni = true;
					}
				}

				if (auto *stacks = meta_object->if_contains("stacks")) {
					for (const auto &[category, stack]: stacks->as_object()) {
						out.stackCategories[Identifier(category)] = JSON::value_to<Identifier>(stack);
					}
				}
			}
		}

		std::unordered_set<std::string> is_tall;

		for (const auto &[name, json]: json_map) {
			std::filesystem::path png_path = base_dir / name / "tile.png";
			int width{}, height{}, channels{};
			images.emplace(name, stbi_load(png_path.string().c_str(), &width, &height, &channels, 4));

			Identifier tilename = JSON::value_to<Identifier>(json.at("tilename"));

			int desired_dimension = 16;

			if (auto autotile_iter = out.autotileSetMap.find(tilename); autotile_iter != out.autotileSetMap.end()) {
				short_autotiles.insert(name);
				out.marchableMap[tilename] = MarchableInfo{tilename, autotile_iter->second, false};
				desired_dimension = 64;
			} else {
				non_autotiles.insert(name);
			}

			if (json.at("solid").as_bool()) {
				out.solid.insert(tilename);
			}

			if (json.at("land").as_bool()) {
				out.land.insert(tilename);
			}

			if (channels != 3 && channels != 4) {
				throw std::runtime_error("Invalid channel count for " + name + ": " + std::to_string(channels) + " (expected 3 or 4)");
			}

			if (desired_dimension == 16 && width == 16 && height == 32) {
				is_tall.insert(name);
			} else if (desired_dimension == 64 && width == 64 && height == 128) {
				short_autotiles.erase(name); // lazy tbh
				tall_autotiles.insert(name);
				out.marchableMap[tilename].tall = true;
			} else {
				if (width != desired_dimension) {
					throw std::runtime_error("Invalid width for " + name + ": " + std::to_string(width) + " (expected " + std::to_string(desired_dimension) + ')');
				}

				if (height != desired_dimension) {
					throw std::runtime_error("Invalid height for " + name + ": " + std::to_string(height) + " (expected " + std::to_string(desired_dimension) + ')');
				}
			}
		}

		// We want to represent the 4x8 autotile sets as 32 wide, 1 tall lines of tiles.
		// We then want to distribute those lines into a square whose dimension is a power of two,
		// but no larger than necessary to store all the lines plus all the remaining 4x4 autotile
		// sets (which will be 16 wide, 1 tall lines of tiles) and single tiles.
		const size_t autotile_4x8_count = tall_autotiles.size();
		const size_t autotile_4x4_count = short_autotiles.size();
		const size_t single_tile_count  = json_map.size() - autotile_4x8_count - autotile_4x4_count;
		// For the sake of calculation, we can pretend that the tiles not part of autotile sets are
		// arranged into lines just like the autotile sets. We add one because the top left tile of
		// the tileset has to be empty.
		const size_t effective_4x8_autotile_sets = 1 + autotile_4x8_count + updiv(autotile_4x4_count, 2) + updiv(single_tile_count + is_tall.size(), 32);
		// If the effective number of lines is no more than 32, then the square will be 32x32 tiles.
		size_t dimension = 32;
		// Otherwise, we need to take the square root of the effective number of lines and round it
		// up to the nearest power of two. This will get us the final side length of the omnisquare
		// in tiles.
		if (32 < effective_4x8_autotile_sets) {
			dimension = size_t(std::pow(2, std::ceil(std::log2(std::ceil(std::sqrt(32 * effective_4x8_autotile_sets))))));
		}
		// Each tile is 16 pixels by 16 pixels.
		dimension *= tilesize;

		const size_t raw_byte_count = dimension * dimension * 4;
		auto raw = std::make_shared<uint8_t[]>(raw_byte_count); // 4 channels: RGBA

		// In pixels.
		size_t x_index = 512;
		size_t y_index = 0;

		size_t tile_index = 32;

		if (dimension == 512) {
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

		for (const std::string &name: tall_autotiles) {
			hasher += json_map.at(name);
			const auto &source = images.at(name);

			for (size_t row = 0; row < 4; ++row) {
				for (size_t column = 0; column < 4; ++column) {
					for (size_t y = 0; y < tilesize; ++y) {
						for (size_t x = 0; x < tilesize; ++x) {
							const size_t source_x = tilesize * column + x;
							const size_t destination_y = y + y_index;
							size_t source_y = tilesize * (2 * row) + y;
							size_t destination_x = (8 * row + 2 * column + 1) * tilesize + x + x_index;
							std::memcpy(&raw[4 * (destination_x + destination_y * dimension)], &source[4 * (source_x + 4 * tilesize * source_y)], 4 * sizeof(uint8_t));
							source_y += tilesize;
							destination_x -= tilesize;
							std::memcpy(&raw[4 * (destination_x + destination_y * dimension)], &source[4 * (source_x + 4 * tilesize * source_y)], 4 * sizeof(uint8_t));
						}
					}
				}
			}

			Identifier tilename = JSON::value_to<Identifier>(json_map.at(name).at("tilename"));
			out.ids[tilename] = tile_index;
			for (size_t tile_offset = 0; tile_offset < 4 * 8; ++tile_offset) {
				if (tile_offset % 2 == 0) {
					out.uppers[tile_index + tile_offset] = tile_index + tile_offset + 1;
				}
				out.names[tile_index + tile_offset] = tilename;
			}

			next(4 * 8 * tilesize);
		}

		for (const std::string &name: short_autotiles) {
			hasher += json_map.at(name);
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

			Identifier tilename = JSON::value_to<Identifier>(json_map.at(name).at("tilename"));
			out.ids[tilename] = tile_index;
			for (size_t tile_offset = 0; tile_offset < 4 * 4; ++tile_offset) {
				out.names[tile_index + tile_offset] = tilename;
			}

			next(4 * 4 * tilesize);
		}

		for (const std::string &name: non_autotiles) {
			hasher += json_map.at(name);

			const auto &source = images.at(name);
			for (size_t y = 0; y < tilesize; ++y) {
				for (size_t x = 0; x < tilesize; ++x) {
					std::memcpy(&raw[4 * (x_index + x + dimension * (y_index + y))], &source[4 * (tilesize * y + x)], 4 * sizeof(uint8_t));
				}
			}

			if (!is_tall.contains(name)) {
				Identifier tilename = JSON::value_to<Identifier>(json_map.at(name).at("tilename"));
				out.ids[tilename] = tile_index;
				out.names[tile_index] = std::move(tilename);

				next(tilesize);
				continue;
			}

			Identifier tilename = JSON::value_to<Identifier>(json_map.at(name).at("tilename"));
			out.ids[tilename] = tile_index + 1;
			out.names[tile_index + 1] = tilename;
			out.uppers[tile_index + 1] = tile_index;
			out.names[tile_index] = std::move(tilename);
			next(tilesize);

			for (size_t y = 0; y < tilesize; ++y) {
				for (size_t x = 0; x < tilesize; ++x) {
					std::memcpy(&raw[4 * (x_index + x + dimension * (y_index + y))], &source[4 * (tilesize * (y + 16) + x)], 4 * sizeof(uint8_t));
				}
			}

			next(tilesize);
		}

		out.hash = hexString(hasher.value<std::string>(), false);
		out.ids["base:tile/empty"] = 0;
		out.names[0] = "base:tile/empty";

		if (png_out != nullptr) {
			for (const std::string &name: tall_autotiles) {
				INFO("{} → {}", name, out.ids[JSON::value_to<Identifier>(json_map.at(name).at("tilename"))]);
			}

			for (const std::string &name: short_autotiles) {
				INFO("{} → {}", name, out.ids[JSON::value_to<Identifier>(json_map.at(name).at("tilename"))]);
			}

			for (const std::string &name: non_autotiles) {
				INFO("{} → {}", name, out.ids[JSON::value_to<Identifier>(json_map.at(name).at("tilename"))]);
			}

			std::stringstream ss;

			stbi_write_png_to_func(+[](void *context, void *data, int size) {
				std::stringstream &ss = *reinterpret_cast<std::stringstream *>(context);
				ss << std::string_view(reinterpret_cast<const char *>(data), size);
			}, &ss, dimension, dimension, 4, raw.get(), dimension * 4);

			*png_out = ss.str();
		}

		if (side == Side::Client) {
			auto texture = std::make_shared<Texture>(std::move(tileset_name));
			texture->alpha = true;
			texture->filter = GL_NEAREST;
			texture->format = GL_RGBA;
			texture->init(raw, dimension, dimension);

			out.cachedTexture = std::move(texture);
		}

		return out;
	}
}
