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
#include <deque>
#include <numeric>

#ifdef USING_VCPKG
#include "lib/stb/stb_image.h"
#include "lib/stb/stb_image_write.h"
#else
#include "lib/stb/stb_image.h"
#include "lib/stb/stb_image_write.h"
#endif

namespace Game3 {
	Tileset tileStitcher2(const std::filesystem::path &base_dir, Identifier tileset_name, Side side, std::string *png_out) {
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

		std::set<std::string> is_tall;
		std::set<std::string> autotile48;

		for (const auto &[name, json]: json_map) {
			std::filesystem::path png_path = base_dir / name / "tile.png";
			int width{}, height{}, channels{};
			images.emplace(name, stbi_load(png_path.string().c_str(), &width, &height, &channels, 4));

			Identifier tilename = JSON::value_to<Identifier>(json.at("tilename"));

			int desired_dimension = 16;

			if (auto autotile_iter = out.autotileSetMap.find(tilename); autotile_iter != out.autotileSetMap.end()) {
				if (width == 16 * 11 && height == 16 * 5) {
					autotile48.insert(name);
					out.marchableMap[tilename] = MarchableInfo{tilename, autotile_iter->second, false, true};
					desired_dimension = -1;
				} else {
					short_autotiles.insert(name);
					out.marchableMap[tilename] = MarchableInfo{tilename, autotile_iter->second, false, false};
					desired_dimension = 64;
				}
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

			if (desired_dimension == -1) {
				// Already handled above.
			} else if (desired_dimension == 16 && width == 16 && height == 32) {
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

		// We want to represent the 48-tile autotile sets as 48-wide, 1 tall lines of tiles.
		// We want to represent the 4x8 autotile sets as 32 wide, 1 tall lines of tiles.
		// We then want to distribute those lines into a square whose dimension is a power of two,
		// but no larger than necessary to store all the lines plus all the remaining 4x4 autotile
		// sets (which will be 16 wide, 1 tall lines of tiles) and single tiles.
		const size_t autotile_48_count = autotile48.size();
		const size_t autotile_32_count = tall_autotiles.size();
		const size_t autotile_16_count = short_autotiles.size();
		const size_t single_tile_count = json_map.size() - autotile_48_count - autotile_32_count - autotile_16_count;
		const size_t single_tall_count = is_tall.size();
		const size_t single_short_count = single_tile_count - single_tall_count;

		// A "guaranteed combo" is a combination of a 48-wide row with a 16-wide row, forming a single 64-wide row.
		// We want to start with these until we exhaust either the 48-wide rows or 16-wide rows.
		const size_t guaranteed_combos = std::min(autotile_48_count, autotile_16_count);

		// For the sake of calculation, we can pretend that the tiles not part of autotile sets are
		// arranged into lines just like the autotile sets. We add one because the top left tile of
		// the tileset has to be empty. The - 63 is to account for single tiles that we can stick
		// in 63 of the 64 blank spots at the top.
		const size_t effective_64s = 1 + guaranteed_combos + updiv(autotile_32_count, 2) + updiv(autotile_48_count - guaranteed_combos, 2) + updiv(autotile_16_count - guaranteed_combos, 2) + updiv(2 * single_tall_count, 64) + updiv(std::max<int64_t>(0, static_cast<int64_t>(single_short_count) - 63), 64);

		// If the effective number of lines is no more than 64, then the square will be 64x64 tiles.
		size_t dimension = 64;

		// Otherwise, we need to take the square root of the effective number of lines and round it
		// up to the nearest power of two. This will get us the final side length of the omnisquare
		// in tiles.
		if (64 < effective_64s) {
			dimension = static_cast<size_t>(std::pow(2, std::ceil(std::log2(std::ceil(std::sqrt(64 * effective_64s))))));
		}

		// Each tile is 16 pixels by 16 pixels.
		dimension *= tilesize;

		const size_t raw_byte_count = dimension * dimension * 4; // 4 channels: RGBA
		auto raw = std::make_shared<uint8_t[]>(raw_byte_count);

		size_t x_index = 64;
		size_t y_index = 0;
		size_t tile_index = 64;

		if (dimension == 64 * tilesize) {
			x_index = 0;
			y_index = 1;
		}

		size_t x_pixels = x_index * tilesize;
		size_t y_pixels = y_index * tilesize;

		auto next = [&](size_t increment /* tiles */) {
			x_index += increment;
			tile_index += increment;
			assert(x_index * tilesize <= dimension);
			if (x_index * tilesize == dimension) {
				x_index = 0;
				++y_index;
				y_pixels += tilesize;
			}
			x_pixels = x_index * tilesize;
		};

		auto jump = [&](size_t destination_index) {
			tile_index = destination_index;
			x_index = tile_index % (dimension / tilesize);
			y_index = tile_index / (dimension / tilesize);
			x_pixels = x_index * tilesize;
			y_pixels = y_index * tilesize;
		};

		auto place_48 = [&](const std::string &name) {
			assert(x_index % 64 == 0);

			// The 48-autotiles are arranged in source textures of 8 tiles by 8 tiles. The last two rows of 8 tiles are unused.
			constexpr size_t row_tiles = 8;
			hasher += json_map.at(name);
			const auto &source = images.at(name);

			Identifier tilename = JSON::value_to<Identifier>(json_map.at(name).at("tilename"));
			out.ids[tilename] = tile_index;
			for (size_t tile_offset = 0; tile_offset < 48; ++tile_offset) {
				out.names[tile_index + tile_offset] = tilename;
			}

			for (size_t i = 0; i < 48; i += row_tiles) {
				assert(dimension - x_pixels >= 48 * tilesize);
				size_t source_x = tilesize * (i % row_tiles);
				for (size_t row = 0; row < tilesize; ++row) {
					size_t source_y = tilesize * (i / row_tiles) + row;
					std::memmove(&raw[4 * (x_pixels + y_pixels * dimension)], &source[4 * (source_x + source_y * row_tiles * tilesize)], 4 * row_tiles * tilesize * sizeof(uint8_t));
				}
				next(row_tiles);
			}
		};

		auto place_32 = [&](const std::string &name) {
			assert(x_index % 32 == 0);

			hasher += json_map.at(name);
			const auto &source = images.at(name);

			for (size_t row = 0; row < 4; ++row) {
				for (size_t column = 0; column < 4; ++column) {
					// TODO: please optimize this nonsense. We don't need to do a separate loop iteration and memcpy for each pixel. That's absurd.
					for (size_t y = 0; y < tilesize; ++y) {
						for (size_t x = 0; x < tilesize; ++x) {
							const size_t source_x = tilesize * column + x;
							const size_t destination_y = y + y_pixels;
							size_t source_y = tilesize * (2 * row) + y;
							size_t destination_x = (8 * row + 2 * column + 1) * tilesize + x + x_pixels;
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

			next(4 * 8);
		};

		auto place_16 = [&](const std::string &name) {
			assert(x_index % 16 == 0);

			hasher += json_map.at(name);
			const auto &source = images.at(name);

			for (size_t row = 0; row < 4; ++row) {
				for (size_t column = 0; column < 4; ++column) {
					// TODO: optimize here too.
					for (size_t y = 0; y < tilesize; ++y) {
						for (size_t x = 0; x < tilesize; ++x) {
							const size_t source_x = tilesize * column + x;
							const size_t source_y = tilesize * row + y;
							const size_t destination_x = (4 * row + column) * tilesize + x + x_pixels;
							const size_t destination_y = y + y_pixels;
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

			next(4 * 4);
		};

		auto place_tall = [&](const std::string &name) {
			hasher += json_map.at(name);

			const auto &source = images.at(name);
			for (size_t y = 0; y < tilesize; ++y) {
				for (size_t x = 0; x < tilesize; ++x) {
					std::memcpy(&raw[4 * (x_pixels + x + dimension * (y_pixels + y))], &source[4 * (tilesize * y + x)], 4 * sizeof(uint8_t));
				}
			}

			Identifier tilename = JSON::value_to<Identifier>(json_map.at(name).at("tilename"));
			out.ids[tilename] = tile_index + 1;
			out.names[tile_index + 1] = tilename;
			out.uppers[tile_index + 1] = tile_index;
			out.names[tile_index] = std::move(tilename);
			next(1);

			for (size_t y = 0; y < tilesize; ++y) {
				for (size_t x = 0; x < tilesize; ++x) {
					std::memcpy(&raw[4 * (x_pixels + x + dimension * (y_pixels + y))], &source[4 * (tilesize * (y + 16) + x)], 4 * sizeof(uint8_t));
				}
			}

			next(1);
		};

		auto place_short = [&](const std::string &name) {
			hasher += json_map.at(name);

			const auto &source = images.at(name);
			for (size_t y = 0; y < tilesize; ++y) {
				for (size_t x = 0; x < tilesize; ++x) {
					std::memcpy(&raw[4 * (x_pixels + x + dimension * (y_pixels + y))], &source[4 * (tilesize * y + x)], 4 * sizeof(uint8_t));
				}
			}

			Identifier tilename = JSON::value_to<Identifier>(json_map.at(name).at("tilename"));
			out.ids[tilename] = tile_index;
			out.names[tile_index] = std::move(tilename);

			next(1);
		};

		size_t remaining_combos = guaranteed_combos;
		while (remaining_combos-- != 0) {
			place_48(*autotile48.begin());
			place_16(*short_autotiles.begin());
			short_autotiles.erase(short_autotiles.begin());
			autotile48.erase(autotile48.begin());
		}

		std::vector<size_t> free_16_indices;
		free_16_indices.reserve(autotile48.size());

		for (const std::string &name: autotile48) {
			assert(x_index % 64 == 0);
			place_48(name);
			free_16_indices.push_back(tile_index);
			next(16);
		}

		// Maybe I could do something intelligent where when the dimension is at least 128 tiles, I could do two 48s followed by a 32.
		// Or if the dimension is at least 256 tiles, I could do three 48s followed by a 16. And so on and so forth.
		// But that would require *me* to be intelligent and it's 1:47 AM right now.

		size_t last_index = tile_index;

		for (const std::string &name: tall_autotiles) {
			place_32(name);
			last_index = tile_index;
		}

		for (const std::string &name: short_autotiles) {
			if (free_16_indices.empty()) {
				place_16(name);
				last_index = tile_index;
			} else {
				jump(free_16_indices.back());
				free_16_indices.pop_back();
				place_16(name);
				if (free_16_indices.empty()) {
					jump(last_index);
				}
			}
		}

		std::deque<size_t> free_singles(63);
		std::ranges::iota(free_singles, 1);

		std::vector<std::string> shorts;
		std::vector<std::string> talls;
		shorts.reserve(non_autotiles.size() - is_tall.size());
		talls.reserve(is_tall.size());

		while (!non_autotiles.empty()) {
			if (const std::string &name = *non_autotiles.begin(); is_tall.contains(name)) {
				talls.push_back(name);
			} else {
				shorts.push_back(name);
			}
			non_autotiles.erase(non_autotiles.begin());
		}

		if (!shorts.empty()) {
			jump(free_singles.front());
			place_short(shorts.back());
			shorts.pop_back();
			jump(last_index);
		}

		free_singles.pop_front();

		while (!talls.empty()) {
			std::string name = std::move(talls.back());
			talls.pop_back();
			if (free_singles.size() >= 2) {
				jump(free_singles.front());
				free_singles.pop_front();
				free_singles.pop_front();
				place_tall(name);
				jump(last_index);
			} else {
				place_tall(name);
				last_index = tile_index;
			}
		}

		while (!shorts.empty()) {
			std::string name = std::move(shorts.back());
			shorts.pop_back();
			if (!free_singles.empty()) {
				jump(free_singles.front());
				free_singles.pop_front();
				place_short(name);
				jump(last_index);
			} else {
				place_short(name);
			}
		}

		out.hash = hexString(hasher.value<std::string>(), false);
		out.ids["base:tile/empty"] = 0;
		out.names[0] = "base:tile/empty";

		if (png_out != nullptr) {
			std::stringstream ss;

			stbi_write_png_to_func(+[](void *context, void *data, int size) {
				std::stringstream &ss = *reinterpret_cast<std::stringstream *>(context);
				ss << std::string_view(reinterpret_cast<const char *>(data), size);
			}, &ss, dimension, dimension, 4, raw.get(), dimension * 4);

			*png_out = std::move(ss).str();
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
