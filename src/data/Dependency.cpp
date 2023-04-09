#include "data/Datapack.h"
#include "data/Dependency.h"

namespace Game3 {
	bool Dependency::validate(const Datapack &datapack) const {
		return datapack.id == id && range.contains(datapack.version);
	}
}
