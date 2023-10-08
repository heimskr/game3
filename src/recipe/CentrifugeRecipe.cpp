#include "game/FluidContainer.h"
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
		if (auto fluids = std::dynamic_pointer_cast<FluidContainer>(container))
			if (auto iter = fluids->levels.find(input.id); iter != fluids->levels.end())
				return input.amount <= iter->second;

		return false;
	}

	bool CentrifugeRecipe::craft(Game &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) {
		auto fluids = std::dynamic_pointer_cast<FluidContainer>(input_container);
		auto inventory = std::dynamic_pointer_cast<Inventory>(output_container);

		if (!fluids || !inventory || !canCraft(fluids))
			return false;

		auto inventory_lock = inventory->uniqueLock();
		ItemStack output = getOutput(input, game);

		if (!inventory->canInsert(output))
			return false;

		assert(0. <= (fluids->levels.at(input.id) -= input.amount));
		leftovers = inventory->add(output);
		assert(!leftovers.has_value());
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
