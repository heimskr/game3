#pragma once

#include "data/Identifier.h"
#include "graphics/Tileset.h"

#include <filesystem>
#include <string>

namespace Game3 {
	Tileset tileStitcher(const std::filesystem::path &base_dir, Identifier tileset_name, std::string *png_out = nullptr);
}
