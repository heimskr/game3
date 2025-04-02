#pragma once

#include <string>

#include "data/Range.h"
#include "data/SemanticVersion.h"

namespace Game3 {
	class Datapack;

	struct Dependency {
		std::string id;
		Range<SemanticVersion> range;

		bool validate(const Datapack &) const;
	};
}
