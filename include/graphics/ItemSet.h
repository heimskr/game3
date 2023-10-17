#pragma once

#include "types/Types.h"
#include "graphics/ItemTexture.h"
#include "registry/Registerable.h"

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Game;
	class ItemStack;
	class Texture;
	struct ItemTextureRegistry;

	class ItemSet: public NamedRegisterable {
		public:
			std::shared_ptr<Texture> texture;

		private:
			ItemSet(Identifier identifier_);

			std::string name;
			std::string hash;
			std::shared_ptr<Texture> cachedTexture;

		friend ItemSet itemStitcher(ItemTextureRegistry *, const std::filesystem::path &, Identifier, std::string *);
	};

	using ItemSetPtr = std::shared_ptr<ItemSet>;
}
