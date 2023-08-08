#pragma once

#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "data/Dependency.h"

namespace Game3 {
	class Datapack {
		public:
			std::string id;
			Version version;
			std::vector<Dependency> dependencies;
	};
}
