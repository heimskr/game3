#pragma once

#include <nlohmann/json.hpp>

#include "Types.h"
#include "item/Item.h"

namespace Game3 {
	class Game;

	class Crop: public NamedRegisterable {
		public:
			std::vector<Identifier> stages;
			ItemStack product;
			double chance;

			Crop(Identifier, std::vector<Identifier> stages_, ItemStack product_, double chance_);
			Crop(Identifier, Game &, const nlohmann::json &);
	};
}
