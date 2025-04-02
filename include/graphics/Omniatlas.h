#pragma once

#include "data/Identifier.h"
#include "graphics/Texture.h"

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace Game3 {
	struct ItemTextureRegistry;
	struct ResourceRegistry;

	class Omniatlas {
		public:
			TexturePtr texture;

			Omniatlas(const std::filesystem::path &, ItemTextureRegistry * = nullptr, ResourceRegistry * = nullptr);
	};
}
