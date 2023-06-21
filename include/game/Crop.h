#pragma once

#include <nlohmann/json.hpp>

#include "registry/Registerable.h"

namespace Game3 {
	class Crop: public NamedRegisterable {
		public:
			std::vector<Identifier> stages;
			double chance;

			Crop(Identifier, std::vector<Identifier> stages_, double chance_);
			Crop(Identifier, const nlohmann::json &);
	};
}
