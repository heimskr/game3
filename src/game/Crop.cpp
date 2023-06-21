#include <cassert>

#include "game/Crop.h"

namespace Game3 {
	Crop::Crop(Identifier identifier_, std::vector<Identifier> stages_, double chance_):
	NamedRegisterable(std::move(identifier_)), stages(std::move(stages_)), chance(chance_) {
		assert(!stages.empty());
	}

	Crop::Crop(Identifier identifier_, const nlohmann::json &json):
		Crop(std::move(identifier_), json.at("stages").get<decltype(stages)>(), json.at("chance").get<double>()) {}
}
