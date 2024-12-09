#pragma once

#include <string>
#include <vector>

#include <boost/json/fwd.hpp>

#include "data/Dependency.h"

namespace Game3 {
	class Datapack {
		public:
			std::string id;
			Version version;
			std::vector<Dependency> dependencies;
	};
}
