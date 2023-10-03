#include "game/Crop.h"
#include "game/Game.h"

#include <cassert>

namespace Game3 {
	Crop::Crop(Identifier identifier_, Identifier custom_type, std::vector<Identifier> stages_, Products products_, double chance_, nlohmann::json custom_data):
	NamedRegisterable(std::move(identifier_)),
	customType(std::move(custom_type)),
	stages(std::move(stages_)),
	products(std::move(products_)),
	chance(chance_),
	customData(std::move(custom_data)) {
		assert(!stages.empty());
	}

	Crop::Crop(Identifier identifier_, Game &game, const nlohmann::json &json):
		Crop(std::move(identifier_), getCustomType(json), json.at("stages"), Products::fromJSON(game, json.at("products")), json.at("chance"), getCustomData(json)) {}

	Identifier Crop::getCustomType(const nlohmann::json &json) {
		if (auto iter = json.find("type"); iter != json.end())
			return iter->get<Identifier>();
		return {};
	}

	nlohmann::json Crop::getCustomData(const nlohmann::json &json) {
		if (auto iter = json.find("custom"); iter != json.end())
			return *iter;
		return {};
	}
}
