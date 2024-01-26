#include "data/ConsumptionRule.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	ConsumptionRule::ConsumptionRule(const Game &, const nlohmann::json &json) {
		input = json.at("in");
		laborOut = json.at("laborOut");
		always = json.at("always");

		if (auto iter = json.find("laborMin"); iter != json.end())
			laborRange.first = *iter;

		if (auto iter = json.find("laborMax"); iter != json.end())
			laborRange.second = *iter;
	}

	ConsumptionRule ConsumptionRule::fromJSON(const Game &game, const nlohmann::json &json) {
		return ConsumptionRule(game, json);
	}

	void to_json(nlohmann::json &json, const ConsumptionRule &rule) {
		json["in"] = rule.getInput();
		json["laborOut"] = rule.getLaborOut();
		json["always"] = rule.getAlways();

		const auto [min, max] = rule.getLaborRange();

		if (-std::numeric_limits<double>::infinity() < min)
			json["laborMin"] = min;

		if (max < std::numeric_limits<double>::infinity())
			json["laborMax"] = max;
	}
}
