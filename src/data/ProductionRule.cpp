#include "data/ProductionRule.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	ProductionRule::ProductionRule(const Game &game, const nlohmann::json &json) {
		if (auto iter = json.find("in"); iter != json.end())
			inputs = ItemStack::manyFromJSON(game, *iter);
		output = ItemStack::fromJSON(game, json["out"]);
		labor  = json["labor"];
		if (auto iter = json.find("cap"); iter != json.end())
			cap = *iter;
		if (auto iter = json.find("biomes"); iter != json.end())
			biomes = *iter;
		if (auto iter = json.find("randomRange"); iter != json.end())
			randomRange = *iter;
	}

	ProductionRule ProductionRule::fromJSON(const Game &game, const nlohmann::json &json) {
		return ProductionRule(game, json);
	}

	void to_json(nlohmann::json &json, const ProductionRule &rule) {
		if (!rule.inputs.empty())
			json["in"]  = rule.inputs;

		json["out"] = rule.output;
		json["labor"] = rule.labor;

		if (rule.cap)
			json["cap"] = *rule.cap;

		if (rule.biomes)
			json["biomes"] = *rule.biomes;

		if (rule.randomRange)
			json["randomRange"] = *rule.randomRange;
	}
}
