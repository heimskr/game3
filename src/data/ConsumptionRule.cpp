#include "data/ConsumptionRule.h"
#include "lib/JSON.h"

namespace Game3 {
	namespace {
		double getNumber(const boost::json::value &json) {
			if (json.is_string()) {
				if (json == "inf") {
					return std::numeric_limits<LaborAmount>::infinity();
				}

				if (json == "-inf") {
					return -std::numeric_limits<LaborAmount>::infinity();
				}

				throw std::invalid_argument("Invalid laborMax: " + boost::json::serialize(json));
			}

			return getDouble(json);
		}
	}

	ConsumptionRule::ConsumptionRule(const std::shared_ptr<Game> &, const boost::json::value &json) {
		const auto &object = json.as_object();

		input = boost::json::value_to<Identifier>(object.at("in"));
		laborOut = boost::json::value_to<LaborAmount>(object.at("laborOut"));
		always = object.at("always").as_bool();

		if (auto iter = object.find("laborMin"); iter != object.end()) {
			laborRange.first = getNumber(iter->value());
		}

		if (auto iter = object.find("laborMax"); iter != object.end()) {
			laborRange.second = getNumber(iter->value());
		}

		if (auto iter = object.find("rate"); iter != object.end()) {
			rate = getDouble(iter->value());
		}

		if (auto iter = object.find("ignoreLabor"); iter != object.end()) {
			ignoreLabor = iter->value().as_bool();
		}
	}

	ConsumptionRule tag_invoke(boost::json::value_to_tag<ConsumptionRule>, const boost::json::value &json, const GamePtr &game) {
		return {game, json};
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const ConsumptionRule &rule) {
		auto &object = json.emplace_object();

		object["in"] = boost::json::value_from(rule.getInput());
		object["laborOut"] = rule.getLaborOut();
		object["always"] = rule.getAlways();

		const auto [min, max] = rule.getLaborRange();

		if (-std::numeric_limits<double>::infinity() < min) {
			object["laborMin"] = min;
		}

		if (max < std::numeric_limits<double>::infinity()) {
			object["laborMax"] = max;
		}

		if (auto rate = rule.getRate(); rate != 1.0) {
			object["rate"] = rate;
		}

		if (auto ignore = rule.getIgnoreLabor()) {
			object["ignoreLabor"] = ignore;
		}
	}
}
