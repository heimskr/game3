#include "game/FluidContainer.h"
#include "game/Game.h"
#include "game/HasFluids.h"
#include "game/Inventory.h"
#include "recipe/LiquifierRecipe.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

namespace Game3 {
	LiquifierRecipe::LiquifierRecipe(Input input_, Output output_):
		input(std::move(input_)), output(std::move(output_)) {}

	LiquifierRecipe::Input LiquifierRecipe::getInput(const GamePtr &) {
		return input;
	}

	LiquifierRecipe::Output LiquifierRecipe::getOutput(const Input &, const GamePtr &) {
		return output;
	}

	bool LiquifierRecipe::canCraft(const std::shared_ptr<Container> &container) {
		if (auto inventory = std::dynamic_pointer_cast<Inventory>(container))
			return inventory->contains(input);

		return false;
	}

	bool LiquifierRecipe::craft(const GamePtr &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) {
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

	bool LiquifierRecipe::craft(const GamePtr &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) {
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

	void LiquifierRecipe::toJSON(nlohmann::json &json) const {
		json["type"] = LiquifierRecipeRegistry::ID();
		json["input"] = input;
		json["output"] = output;
	}

	LiquifierRecipe LiquifierRecipe::fromJSON(const GamePtr &game, const nlohmann::json &json) {
		LiquifierRecipe recipe;
		recipe.input = ItemStack::fromJSON(game, json.at("input"));
		recipe.output = FluidStack::fromJSON(*game, json.at("output"));
		return recipe;
	}
}
