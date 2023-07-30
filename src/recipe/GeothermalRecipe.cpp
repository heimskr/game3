#include "game/EnergyContainer.h"
#include "game/FluidContainer.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "recipe/GeothermalRecipe.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

namespace Game3 {
	GeothermalRecipe::GeothermalRecipe(Input input_, Output output_):
		input(std::move(input_)), output(output_) {}

	GeothermalRecipe::Input GeothermalRecipe::getInput(Game &) {
		return input;
	}

	GeothermalRecipe::Output GeothermalRecipe::getOutput(const Input &input_, Game &) {
		return input_.id == input.id && input.amount <= input_.amount? output : 0;
	}

	bool GeothermalRecipe::canCraft(const std::shared_ptr<Container> &container) {
		if (auto fluids = std::dynamic_pointer_cast<FluidContainer>(container))
			if (auto iter = fluids->levels.find(input.id); iter != fluids->levels.end())
				return input.amount <= iter->second;

		return false;
	}

	bool GeothermalRecipe::craft(Game &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) {
		auto fluids = std::dynamic_pointer_cast<FluidContainer>(input_container);
		auto energy = std::dynamic_pointer_cast<EnergyContainer>(output_container);

		if (!fluids || !energy || !canCraft(fluids))
			return false;

		EnergyAmount output = getOutput(input, game);

		if (!energy->canInsert(output))
			return false;

		assert(energy->add(output) == 0);
		leftovers.reset();
		return true;
	}

	GeothermalRecipe GeothermalRecipe::fromJSON(const Game &game, const nlohmann::json &json) {
		return {FluidStack::fromJSON(game, json.at("input")), json.at("output")};
	}

	void to_json(nlohmann::json &json, const GeothermalRecipe &recipe) {
		json["input"] = recipe.input;
		json["output"] = recipe.output;
	}
}
