#include <cassert>

#include "game/Crop.h"
#include "game/Game.h"

namespace Game3 {
	Crop::Crop(Identifier identifier_, std::vector<Identifier> stages_, ItemStack product_, double chance_):
	NamedRegisterable(std::move(identifier_)), stages(std::move(stages_)), product(std::move(product_)), chance(chance_) {
		assert(!stages.empty());
	}

	Crop::Crop(Identifier identifier_, Game &game, const nlohmann::json &json):
		Crop(std::move(identifier_), json.at("stages"), ItemStack::fromJSON(game, json.at("product")), json.at("chance")) {}
}
