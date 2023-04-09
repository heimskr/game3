#include "registry/Registry.h"

namespace Game3 {
	void from_json(const nlohmann::json &, Registry &) {
		throw std::logic_error("Cannot serialize a Registry");
	}
}
