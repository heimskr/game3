#pragma once

#include "graphics/Tileset.h"

#include <filesystem>

namespace Game3 {
	Tileset stitcher(const std::filesystem::path &base_dir, Identifier tileset_name);
}
