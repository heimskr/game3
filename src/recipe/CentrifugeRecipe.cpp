#include "game/FluidContainer.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "recipe/CentrifugeRecipe.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

namespace Game3 {
	CentrifugeRecipe::CentrifugeRecipe(Input input, std::unordered_map<boost::json::value, double> weight_map):
		input(input), weightMap(std::move(weight_map)) {}

	CentrifugeRecipe::Input CentrifugeRecipe::getInput(const GamePtr &) {
		return input;
	}

	CentrifugeRecipe::Output CentrifugeRecipe::getOutput(const Input &, const GamePtr &game) {
		return boost::json::value_to<ItemStackPtr>(weightedChoice(weightMap, threadContext.rng), game);
	}

	bool CentrifugeRecipe::canCraft(const std::shared_ptr<Container> &container) {
		if (auto fluids = std::dynamic_pointer_cast<FluidContainer>(container)) {
			if (auto iter = fluids->levels.find(input.id); iter != fluids->levels.end()) {
				return input.amount <= iter->second;
			}
		}

		return false;
	}

	bool CentrifugeRecipe::craft(const GamePtr &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) {
		auto fluids = std::dynamic_pointer_cast<FluidContainer>(input_container);
		auto inventory = std::dynamic_pointer_cast<Inventory>(output_container);

		if (!fluids || !inventory || !canCraft(fluids)) {
			return false;
		}

		ItemStackPtr output = getOutput(input, game);

		if (!inventory->canInsert(output)) {
			return false;
		}

		auto remaining = fluids->levels.at(input.id) -= input.amount;

		assert(0. <= remaining);
		leftovers = inventory->add(output);
		assert(!*leftovers);
		leftovers.reset();
		return true;
	}

	bool CentrifugeRecipe::craft(const GamePtr &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) {
		std::optional<Output> leftovers;
		const bool result = craft(game, input_container, output_container, leftovers);
		assert(!leftovers);
		return result;
	}

	void CentrifugeRecipe::toJSON(boost::json::value &json, const GamePtr &game) const {
		auto &object = json.emplace_object();
		object["type"] = boost::json::value_from(CentrifugeRecipeRegistry::ID());
		object["input"] = boost::json::value_from(input, game);
		auto &output = object["output"].emplace_array();
		for (const auto &[item, weight]: weightMap) {
			output.emplace_back(boost::json::value{weight, item});
		}
	}

	CentrifugeRecipe tag_invoke(boost::json::value_to_tag<CentrifugeRecipe>, const boost::json::value &json, const GamePtr &game) {
		CentrifugeRecipe recipe;

		recipe.input = boost::json::value_to<FluidStack>(json.at("input"), game);

		for (const auto &item: json.at("output").as_array()) {
			recipe.weightMap[item.at(1)] = boost::json::value_to<double>(item.at(0));
		}

		return recipe;
	}
}
