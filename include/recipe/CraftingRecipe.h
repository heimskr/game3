#pragma once

#include <nlohmann/json_fwd.hpp>

#include "data/Identifier.h"
#include "item/Item.h"
#include "recipe/CraftingRequirement.h"
#include "recipe/Recipe.h"
#include "registry/Registries.h"

namespace Game3 {
	struct CraftingRecipe: Recipe<std::vector<CraftingRequirement>, std::vector<ItemStack>> {
		Input input;
		Output output;
		Identifier stationType;

		CraftingRecipe() = default;
		CraftingRecipe(Input, Output, Identifier);

		Input getInput(Game &) override;
		Output getOutput(const Input &, Game &) override;
		bool canCraft(const std::shared_ptr<Container> &) override;
		bool craft(Game &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) override;

		static CraftingRecipe fromJSON(const Game &, const nlohmann::json &);
	};

	// void from_json(const nlohmann::json &, CraftingRecipe &);
	void to_json(nlohmann::json &, const CraftingRecipe &);

	struct CraftingRecipeRegistry: UnnamedJSONRegistry<CraftingRecipe> {
		static Identifier ID() { return {"base", "crafting_recipe"}; }
		CraftingRecipeRegistry(): UnnamedJSONRegistry(ID()) {}
	};
}
