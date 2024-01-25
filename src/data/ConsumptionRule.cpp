#include "data/ConsumptionRule.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	ConsumptionRule::ConsumptionRule(const Game &game, const nlohmann::json &json) {
		input = json.at("in");
		laborOut = json.at("laborOut");
		always = json.at("always");
	}

	ConsumptionRule ConsumptionRule::fromJSON(const Game &game, const nlohmann::json &json) {
		return ConsumptionRule(game, json);
	}

	void to_json(nlohmann::json &json, const ConsumptionRule &rule) {
		json["in"] = rule.getInput();
		json["laborOut"] = rule.getLaborOut();
		json["always"] = rule.getAlways();
	}
}
