#pragma once

#include "data/Identifier.h"
#include "graphics/Tileset.h"

#include <filesystem>
#include <string>

namespace Game3 {
	Tileset tileStitcher(const std::filesystem::path &base_dir, Identifier tileset_name, Side, std::string *png_out = nullptr);
	Tileset tileStitcher1(const std::filesystem::path &base_dir, Identifier tileset_name, Side, std::string *png_out = nullptr);
	Tileset tileStitcher2(const std::filesystem::path &base_dir, Identifier tileset_name, Side, std::string *png_out = nullptr);
}
