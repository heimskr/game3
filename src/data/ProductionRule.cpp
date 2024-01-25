#include "data/ProductionRule.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	ProductionRule::ProductionRule(const Game &game, const nlohmann::json &json) {
		output = ItemStack::fromJSON(game, json["out"]);
		labor  = json["labor"];

		if (auto iter = json.find("in"); iter != json.end())
			inputs = ItemStack::manyFromJSON(game, *iter);

		if (auto iter = json.find("cap"); iter != json.end())
			cap = *iter;

		if (auto iter = json.find("biomes"); iter != json.end())
			biomes = *iter;

		if (auto iter = json.find("randomRange"); iter != json.end())
			randomRange = *iter;

		if (auto iter = json.find("richnessEffect"); iter != json.end())
			richnessEffect = *iter;
	}

	bool ProductionRule::doesBiomeMatch(BiomeType biome) const {
		return !biomes || biomes->contains(biome);
	}

	ProductionRule ProductionRule::fromJSON(const Game &game, const nlohmann::json &json) {
		return ProductionRule(game, json);
	}

	void to_json(nlohmann::json &json, const ProductionRule &rule) {
		json["out"] = rule.getOutput();
		json["labor"] = rule.getLabor();

		if (auto &inputs = rule.getInputs(); !inputs.empty())
			json["in"]  = inputs;

		if (auto cap = rule.getCap())
			json["cap"] = *cap;

		if (auto &biomes = rule.getBiomes())
			json["biomes"] = *biomes;

		if (auto &range = rule.getRandomRange())
			json["randomRange"] = *range;

		if (auto effect = rule.getRichnessEffect())
			json["richnessEffect"] = *effect;
	}
}
