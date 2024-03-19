#include "game/FluidContainer.h"
#include "game/Game.h"
#include "game/HasFluids.h"
#include "game/Inventory.h"
#include "recipe/BiomassLiquefierRecipe.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

namespace Game3 {
	BiomassLiquefierRecipe::BiomassLiquefierRecipe(Input input_, Output output_):
		Recipe(input_->getID()), input(std::move(input_)), output(std::move(output_)) {}

	BiomassLiquefierRecipe::Input BiomassLiquefierRecipe::getInput(const GamePtr &) {
		return input;
	}

	BiomassLiquefierRecipe::Output BiomassLiquefierRecipe::getOutput(const Input &, const GamePtr &) {
		return output;
	}

	bool BiomassLiquefierRecipe::canCraft(const std::shared_ptr<Container> &container) {
		if (auto inventory = std::dynamic_pointer_cast<Inventory>(container))
			return inventory->contains(input);

		return false;
	}

	bool BiomassLiquefierRecipe::craft(const GamePtr &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) {
		auto inventory = std::dynamic_pointer_cast<Inventory>(input_container);
		auto fluids = std::dynamic_pointer_cast<FluidContainer>(output_container);

		if (!fluids || !inventory || !canCraft(inventory))
			return false;

		const FluidID liquid_biomass_id = game->getFluid("base:fluid/liquid_biomass")->registryID;

		std::shared_ptr<HasFluids> fluids_owner = fluids->getOwner();
		FluidAmount max_level = fluids_owner->getMaxLevel(liquid_biomass_id);
		if (max_level == 0)
			return false;

		// If this fluid type isn't present in the container and it already has the maximum number of fluid types it can, we can't craft this.
		auto iter = fluids->levels.find(liquid_biomass_id);
		if (iter == fluids->levels.end() && fluids->levels.size() == fluids_owner->getMaxFluidTypes())
			return false;

		FluidAmount &level = iter == fluids->levels.end()? fluids->levels[liquid_biomass_id] : iter->second;

		if (level == max_level)
			return false;

		const FluidAmount sum = level + output;
		if (sum > max_level)
			return false;

		inventory->remove(input);

		// TODO: handle integer overflow...?
		level += output;
		if (level > max_level) {
			leftovers.emplace(level - max_level);
			level = max_level;
		} else {
			leftovers.reset();
		}

		return true;
	}

	// TODO: deduplicate above and below methods.

	bool BiomassLiquefierRecipe::craft(const GamePtr &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) {
		auto inventory = std::dynamic_pointer_cast<Inventory>(input_container);
		auto fluids = std::dynamic_pointer_cast<FluidContainer>(output_container);

		if (!fluids || !inventory || !canCraft(inventory))
			return false;

		const FluidID liquid_biomass_id = game->getFluid("base:fluid/liquid_biomass")->registryID;

		std::shared_ptr<HasFluids> fluids_owner = fluids->getOwner();
		FluidAmount max_level = fluids_owner->getMaxLevel(liquid_biomass_id);
		if (max_level == 0)
			return false;

		// If this fluid type isn't present in the container and it already has the maximum number of fluid types it can, we can't craft this.
		auto iter = fluids->levels.find(liquid_biomass_id);
		if (iter == fluids->levels.end() && fluids->levels.size() == fluids_owner->getMaxFluidTypes())
			return false;

		FluidAmount &level = iter == fluids->levels.end()? fluids->levels[liquid_biomass_id] : iter->second;

		if (level == max_level)
			return false;

		const FluidAmount sum = level + output;
		if (sum > max_level)
			return false;

		inventory->remove(input);
		level = sum;
		return true;
	}

	void BiomassLiquefierRecipe::toJSON(nlohmann::json &json) const {
		json = output;
	}
}
