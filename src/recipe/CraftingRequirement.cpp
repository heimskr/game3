#include "game/Game.h"
#include "recipe/CraftingRequirement.h"

#include <boost/json.hpp>

namespace Game3 {
	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const CraftingRequirement &requirement) {
		if (requirement.is<AttributeRequirement>()) {
			const auto &[attribute, count] = requirement.get<AttributeRequirement>();
			auto &array = json.emplace_array();
			array.emplace_back(boost::json::value_from(attribute));
			array.emplace_back(count);
		} else {
			boost::json::value_from(requirement.get<ItemStackPtr>(), json);
		}
	}

	CraftingRequirement tag_invoke(boost::json::value_to_tag<CraftingRequirement>, const boost::json::value &json, const GamePtr &game) {
		auto id = boost::json::value_to<Identifier>(json.at(0));
		if (id.getPath() == "attribute") {
			const auto &array = json.as_array();
			return AttributeRequirement{std::move(id), array.size() < 2? 1 : boost::json::value_to<ItemCount>(array.at(1))};
		}
		return boost::json::value_to<ItemStackPtr>(json, game);
	}
}
