#include "recipe/CraftingRequirement.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	CraftingRequirement CraftingRequirement::fromJSON(const Game &game, const nlohmann::json &json) {
		Identifier id = json.at(0);
		if (id.getPath() == "attribute")
			return AttributeRequirement{std::move(id), json.size() == 0? 1 : json.at(1).get<ItemCount>()};
		return ItemStack::fromJSON(game, json);
	}

	void to_json(nlohmann::json &json, const CraftingRequirement &requirement) {
		if (requirement.is<AttributeRequirement>()) {
			const auto &[attribute, count] = requirement.get<AttributeRequirement>();
			json[0] = attribute;
			json[1] = count;
		} else {
			json = requirement.get<ItemStack>();
		}
	}
}