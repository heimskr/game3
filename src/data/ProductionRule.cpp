#include "data/ProductionRule.h"
#include "item/Item.h"

#include <boost/json.hpp>

namespace Game3 {
	ProductionRule::ProductionRule(const std::shared_ptr<Game> &game, const boost::json::value &json) {
		output = boost::json::value_to<ItemStackPtr>(json.at("out"), game);
		labor  = boost::json::value_to<LaborAmount>(json.at("labor"));

		if (const auto *object = json.if_object()) {
			if (auto iter = object->find("in"); iter != object->end()) {
				inputs = ItemStack::manyFromJSON(game, iter->value());
			}

			if (auto iter = object->find("cap"); iter != object->end()) {
				cap = iter->value().as_double();
			}

			if (auto iter = object->find("biomes"); iter != object->end()) {
				biomes = boost::json::value_to<std::set<BiomeType>>(iter->value());
			}

			if (auto iter = object->find("randomRange"); iter != object->end()) {
				if (const auto *array = iter->value().if_array()) {
					randomRange.emplace((*array)[0].as_double(), array->at(1).as_double());
				}
			}

			if (auto iter = object->find("richnessEffect"); iter != object->end()) {
				richnessEffect = iter->value().as_double();
			}
		}
	}

	bool ProductionRule::doesBiomeMatch(BiomeType biome) const {
		return !biomes || biomes->contains(biome);
	}

	ProductionRule tag_invoke(boost::json::value_to_tag<ProductionRule>, const boost::json::value &json, const std::shared_ptr<Game> &game) {
		return {game, json};
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const ProductionRule &rule) {
		auto &object = json.emplace_object();
		object["out"] = boost::json::value_from(*rule.getOutput());
		object["labor"] = rule.getLabor();

		if (const auto &inputs = rule.getInputs(); !inputs.empty()) {
			auto &array = object["in"].emplace_array();
			for (const ItemStackPtr &input: inputs) {
				array.emplace_back(boost::json::value_from(*input));
			}
		}

		if (auto cap = rule.getCap()) {
			object["cap"] = *cap;
		}

		if (auto &biomes = rule.getBiomes()) {
			object["biomes"] = boost::json::value_from(*biomes);
		}

		if (auto &range = rule.getRandomRange()) {
			object["randomRange"] = {range->first, range->second};
		}

		if (auto effect = rule.getRichnessEffect()) {
			object["richnessEffect"] = *effect;
		}
	}
}
