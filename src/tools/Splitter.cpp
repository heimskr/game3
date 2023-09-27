#include "config.h"
#include "tools/Flasker.h"
#include "Log.h"
#include "graphics/Tileset.h"
#include "util/FS.h"
#include "util/Util.h"

#include <fstream>
#include <nlohmann/json.hpp>

#ifdef USING_VCPKG
#include <stb_image.h>
#include <stb_image_write.h>
#else
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#endif

namespace Game3 {
	void splitter() {
		std::filesystem::path filename = "resources/tileset.png";

		int width{};
		int height{};
		int channels{};

		auto raw_tiles = std::unique_ptr<uint8_t[], FreeDeleter>(stbi_load(filename.c_str(), &width, &height, &channels, 0));
		if (!raw_tiles)
			throw std::runtime_error("Couldn't load " + filename.string());

		auto json = nlohmann::json::parse(readFile("gamedata/monomap.json"))["data"][0][1]["base:tileset/monomap"];

		Tileset tileset = Tileset::fromJSON("base:tileset/monomap", json);

		std::unordered_map<Identifier, Identifier> march; // tile => corner

		for (const auto &[category, corner]: json["marchable"].get<std::unordered_map<Identifier, Identifier>>())
			for (const auto &name: tileset.getTilesByCategory(category))
				march[name] = corner;

		for (const auto &[name, id]: tileset.getIDs()) {
			const int tileset_x = (id % 32) * 16;
			const int tileset_y = (id / 32) * 16;
			auto raw_tile = std::make_unique<uint8_t[]>(16 * 16 * channels);
			for (size_t y = 0; y < 16; ++y)
				for (size_t x = 0; x < 16; ++x)
					for (int b = 0; b < channels; ++b)
						raw_tile[(y * 16 + x) * channels + b] = raw_tiles[(tileset_x + x + (tileset_y + y) * width) * channels + b];
			std::string dirname = "split/" + name.name.substr(5);
			std::string tilename = dirname + "/tile.png";
			std::filesystem::create_directories(dirname);
			stbi_write_png(tilename.c_str(), 16, 16, 4, raw_tile.get(), 16 * 4);

			nlohmann::json meta;
			meta["tilename"] = name;
			meta["solid"] = tileset.isSolid(name);
			meta["land"] = tileset.isLand(name);

			if (auto iter = march.find(name); iter != march.end()) {
				const Identifier &corner = iter->second;
				const TileID corner_tile = tileset[corner];
				const int corner_x = corner_tile % 32;
				const int corner_y = corner_tile / 32;
				const int offset_x = tileset_x / 16 - corner_x;
				const int offset_y = tileset_y / 16 - corner_y;
				meta["marchable"]["corner"] = corner;
				meta["marchable"]["offset"] = std::make_pair(offset_x, offset_y);
			}

			for (const auto &category: tileset.getCategories(name)) {
				meta["categories"].push_back(category);
				if (auto stack_name_iter = tileset.stackCategories.find(category); stack_name_iter != tileset.stackCategories.end())
					meta["stack"] = stack_name_iter->second;
			}

			if (auto stack_name_iter = tileset.stackNames.find(name); stack_name_iter != tileset.stackNames.end())
				meta["stack"] = stack_name_iter->second;

			std::ofstream of(dirname + "/tile.json");
			of << meta.dump();
		}
	}
}
