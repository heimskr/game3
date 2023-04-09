#include "data/Identifier.h"

namespace Game3 {
	void from_json(const nlohmann::json &json, Identifier &identifier) {
		identifier.space = json.at(0);
		identifier.name  = json.at(1);
	}

	void to_json(nlohmann::json &json, const Identifier &identifier) {
		json[0] = identifier.space;
		json[1] = identifier.name;
	}
}
