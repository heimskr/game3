#include "data/ConsumptionRule.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	namespace {
		double getNumber(const nlohmann::json &json) {
			if (json.is_string()) {
				if (json == "inf")
					return std::numeric_limits<LaborAmount>::infinity();
				if (json == "-inf")
					return -std::numeric_limits<LaborAmount>::infinity();
				throw std::invalid_argument("Invalid laborMax: " + json.dump());
			}

			return json;
		}
	}

	ConsumptionRule::ConsumptionRule(const Game &, const nlohmann::json &json) {
		input = json.at("in");
		laborOut = json.at("laborOut");
		always = json.at("always");

		if (auto iter = json.find("laborMin"); iter != json.end())
			laborRange.first = getNumber(*iter);

		if (auto iter = json.find("laborMax"); iter != json.end())
			laborRange.second = getNumber(*iter);

		if (auto iter = json.find("rate"); iter != json.end())
			rate = *iter;

		if (auto iter = json.find("ignoreLabor"); iter != json.end())
			ignoreLabor = *iter;
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

		if (auto rate = rule.getRate(); rate != 1.0)
			json["rate"] = rate;

		if (auto ignore = rule.getIgnoreLabor())
			json["ignoreLabor"] = ignore;
	}
}
