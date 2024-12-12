#pragma once

#include "data/Identifier.h"
#include "graphics/Texture.h"

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace Game3 {
	class Omniatlas {
		public:
			TexturePtr texture;

			Omniatlas(const std::filesystem::path &);
	};
}
