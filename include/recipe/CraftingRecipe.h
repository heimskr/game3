#pragma once

#include "data/Identifier.h"
#include "item/Item.h"
#include "recipe/Recipe.h"

namespace Game3 {
	struct CraftingRecipe: Recipe<std::vector<ItemStack>, std::vector<ItemStack>> {
		Input input;
		Output output;
		Identifier stationType;

		CraftingRecipe(Input, Output, Identifier);

		Input getInput() override;
		Output getOutput() override;
		bool canCraft(const std::shared_ptr<Container> &) override;
		bool craft(const std::shared_ptr<Container> &, Output &leftovers) override;
	};
}
