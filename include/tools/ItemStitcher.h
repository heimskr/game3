#pragma once

#include "data/Identifier.h"
#include "graphics/ItemSet.h"

#include <filesystem>
#include <string>

namespace Game3 {
	struct ItemTextureRegistry;
	struct ResourceRegistry;

	ItemSet itemStitcher(ItemTextureRegistry *, ResourceRegistry *, const std::filesystem::path &base_dir, Identifier tileset_name, std::string *png_out = nullptr);
}
