#pragma once

#include "data/Identifier.h"
#include "game/Fluids.h"
#include "item/Item.h"
#include "recipe/Recipe.h"
#include "registry/Registries.h"

namespace Game3 {
	struct BiomassLiquefierRecipe: Recipe<ItemStackPtr, FluidAmount, NamedRegisterable> {
		Input input;
		Output output;

		BiomassLiquefierRecipe();
		BiomassLiquefierRecipe(Input, Output);

		Input getInput(const GamePtr &) override;
		Output getOutput(const Input &, const GamePtr &) override;
		/** Doesn't lock the container. */
		bool canCraft(const std::shared_ptr<Container> &) override;
		/** Doesn't lock either container. Fails if there would be any leftover output. */
		bool craft(const GamePtr &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) override;
		/** Doesn't lock either container. */
		bool craft(const GamePtr &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) override;
		void toJSON(boost::json::value &, const GamePtr &) const override;
	};

	struct BiomassLiquefierRecipeRegistry: NamedRegistry<BiomassLiquefierRecipe> {
		static Identifier ID() { return {"base", "registry/biomass_liquefier_recipe"}; }
		BiomassLiquefierRecipeRegistry(): NamedRegistry(ID()) {}
	};
}
