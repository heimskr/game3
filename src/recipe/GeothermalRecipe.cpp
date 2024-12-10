#include "game/EnergyContainer.h"
#include "game/FluidContainer.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "recipe/GeothermalRecipe.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

namespace Game3 {
	GeothermalRecipe::GeothermalRecipe(Input input, Output output):
		input(input), output(output) {}

	GeothermalRecipe::Input GeothermalRecipe::getInput(const GamePtr &) {
		return input;
	}

	GeothermalRecipe::Output GeothermalRecipe::getOutput(const Input &input_, const GamePtr &) {
		return input_.id == input.id && input.amount <= input_.amount? output : 0;
	}

	bool GeothermalRecipe::canCraft(const std::shared_ptr<Container> &container) {
		if (auto fluids = std::dynamic_pointer_cast<FluidContainer>(container))
			if (auto iter = fluids->levels.find(input.id); iter != fluids->levels.end())
				return input.amount <= iter->second;

		return false;
	}

	bool GeothermalRecipe::craft(const GamePtr &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) {
		if (craft(game, input_container, output_container)) {
			leftovers.reset();
			return true;
		}

		return false;
	}

	bool GeothermalRecipe::craft(const GamePtr &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) {
		auto fluids = std::dynamic_pointer_cast<FluidContainer>(input_container);
		auto energy = std::dynamic_pointer_cast<EnergyContainer>(output_container);

		if (!fluids || !energy || !canCraft(fluids))
			return false;

		const EnergyAmount output = getOutput(input, game);

		if (!energy->canInsert(output))
			return false;

		assert(energy->add(output) == 0);
		fluids->levels[input.id] -= input.amount;
		return true;
	}

	void GeothermalRecipe::toJSON(boost::json::value &json, const GamePtr &game) const {
		auto &object = json.emplace_object();
		object["type"] = boost::json::value_from(GeothermalRecipeRegistry::ID());
		object["input"] = boost::json::value_from(input, game);
		object["output"] = boost::json::value_from(output);
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const GeothermalRecipe &recipe, const GamePtr &game) {
		recipe.toJSON(json, game);
	}

	GeothermalRecipe tag_invoke(boost::json::value_to_tag<GeothermalRecipe>, const boost::json::value &json, const std::shared_ptr<Game> &game) {
		return {boost::json::value_to<FluidStack>(json.at("input"), game), boost::json::value_to<EnergyAmount>(json.at("output"))};
	}
}
