#pragma once

#include <string>

#include "data/Range.h"
#include "data/Version.h"

namespace Game3 {
	class Datapack;

	struct Dependency {
		std::string id;
		Range<Version> range;

		bool validate(const Datapack &) const;
	};
}
