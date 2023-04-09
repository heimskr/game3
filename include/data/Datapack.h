#pragma once

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "data/Dependency.h"

namespace Game3 {
	class Datapack {
		public:
			std::string id;
			Version version;
			std::vector<Dependency> dependencies;
	};
}
