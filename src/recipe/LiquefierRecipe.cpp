#include "game/FluidContainer.h"
#include "game/Game.h"
#include "game/HasFluids.h"
#include "game/Inventory.h"
#include "recipe/LiquefierRecipe.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

namespace Game3 {
	LiquefierRecipe::LiquefierRecipe(Input input, Output output):
		input(std::move(input)), output(output) {}

	LiquefierRecipe::Input LiquefierRecipe::getInput(const GamePtr &) {
		return input;
	}

	LiquefierRecipe::Output LiquefierRecipe::getOutput(const Input &, const GamePtr &) {
		return output;
	}

	bool LiquefierRecipe::canCraft(const std::shared_ptr<Container> &container) {
		if (auto inventory = std::dynamic_pointer_cast<Inventory>(container))
			return inventory->contains(input);

		return false;
	}

	bool LiquefierRecipe::craft(const GamePtr &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) {
		auto inventory = std::dynamic_pointer_cast<Inventory>(input_container);
		auto fluids = std::dynamic_pointer_cast<FluidContainer>(output_container);

		if (!fluids || !inventory || !canCraft(inventory))
			return false;


		std::shared_ptr<HasFluids> fluids_owner = fluids->getOwner();
		FluidAmount max_level = fluids_owner->getMaxLevel(output.id);
		if (max_level == 0)
			return false;

		// If this fluid type isn't present in the container and it already has the maximum number of fluid types it can, we can't craft this.
		auto iter = fluids->levels.find(output.id);
		if (iter == fluids->levels.end() && fluids->levels.size() == fluids_owner->getMaxFluidTypes())
			return false;

		FluidAmount &level = iter == fluids->levels.end()? fluids->levels[output.id] : iter->second;

		if (level == max_level)
			return false;

		inventory->remove(input);

		// TODO: handle integer overflow...?
		level += output.amount;
		if (level > max_level) {
			leftovers.emplace(output.id, level - max_level);
			level = max_level;
		} else {
			leftovers.reset();
		}

		return true;
	}

	// TODO: deduplicate above and below methods.

	bool LiquefierRecipe::craft(const GamePtr &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) {
		auto inventory = std::dynamic_pointer_cast<Inventory>(input_container);
		auto fluids = std::dynamic_pointer_cast<FluidContainer>(output_container);

		if (!fluids || !inventory || !canCraft(inventory))
			return false;

		std::shared_ptr<HasFluids> fluids_owner = fluids->getOwner();
		FluidAmount max_level = fluids_owner->getMaxLevel(output.id);
		if (max_level == 0)
			return false;

		// If this fluid type isn't present in the container and it already has the maximum number of fluid types it can, we can't craft this.
		auto iter = fluids->levels.find(output.id);
		if (iter == fluids->levels.end() && fluids->levels.size() == fluids_owner->getMaxFluidTypes())
			return false;

		FluidAmount &level = iter == fluids->levels.end()? fluids->levels[output.id] : iter->second;

		if (level == max_level)
			return false;

		const FluidAmount sum = level + output.amount;
		if (sum > max_level)
			return false;

		inventory->remove(input);
		level = sum;
		return true;
	}

	void LiquefierRecipe::toJSON(boost::json::value &json) const {
		auto &object = json.emplace_object();
		object["type"] = boost::json::value_from(LiquefierRecipeRegistry::ID());
		object["input"] = boost::json::value_from(input);
		object["output"] = boost::json::value_from(output);
	}

	LiquefierRecipe tag_invoke(boost::json::value_to_tag<LiquefierRecipe>, const boost::json::value &json, const GamePtr &game) {
		LiquefierRecipe recipe;
		recipe.input = boost::json::value_to<LiquefierRecipe::Input>(json.at("input"), game);
		recipe.output = boost::json::value_to<LiquefierRecipe::Output>(json.at("output"), game);
		return recipe;
	}
}
