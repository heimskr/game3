#include "game/Game.h"
#include "game/Inventory.h"
#include "recipe/CentrifugeRecipe.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

namespace Game3 {
	CentrifugeRecipe::CentrifugeRecipe(Input input_, std::map<nlohmann::json, double> weight_map):
		input(std::move(input_)), weightMap(std::move(weight_map)) {}

	CentrifugeRecipe::Input CentrifugeRecipe::getInput(Game &) {
		return input;
	}

	CentrifugeRecipe::Output CentrifugeRecipe::getOutput(const Input &, Game &game) {
		return ItemStack::fromJSON(game, weightedChoice(weightMap, threadContext.rng));
	}

	bool CentrifugeRecipe::canCraft(const std::shared_ptr<Container> &container) {
		// auto inventory = std::dynamic_pointer_cast<Inventory>(container);
		// if (!inventory)
		// 	return false;

		// if (auto *stack = input.get_if<ItemStack>()) {
		// 	if (inventory->count(*stack) < stack->count)
		// 		return false;
		// } else if (auto *requirement = input.get_if<AttributeRequirement>()) {
		// 	const auto &[attribute, count] = *requirement;
		// 	if (inventory->countAttribute(attribute) < count)
		// 		return false;
		// } else
		// 	throw std::logic_error("Unhandled CentrifugeRecipe input type: " + std::to_string(input.index()));

		return true;
	}

	bool CentrifugeRecipe::craft(Game &game, const std::shared_ptr<Container> &container, std::optional<Output> &leftovers) {
		// auto inventory = std::dynamic_pointer_cast<Inventory>(container);
		// if (!inventory)
		// 	return false;

		// auto lock = inventory->uniqueLock();

		// if (!canCraft(container))
		// 	return false;

		// inventory->remove(input);
		// leftovers = inventory->add(getOutput(input, game));
		return true;
	}

	CentrifugeRecipe CentrifugeRecipe::fromJSON(const Game &game, const nlohmann::json &json) {
		CentrifugeRecipe recipe;

		recipe.input = FluidStack::fromJSON(game, json.at("input"));

		for (const auto &item: json.at("output"))
			recipe.weightMap[item.at(1)] = item.at(0);

		return recipe;
	}

	void to_json(nlohmann::json &json, const CentrifugeRecipe &recipe) {
		json["input"] = recipe.input;
		std::vector<std::pair<double, nlohmann::json>> weight_pairs;
		for (const auto &[item_json, weight]: recipe.weightMap)
			weight_pairs.emplace_back(weight, item_json);
		json["output"] = std::move(weight_pairs);
	}
}
