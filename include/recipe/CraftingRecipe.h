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

		Input getInput(const std::shared_ptr<Game> &) override;
		Output getOutput(const Input &, const std::shared_ptr<Game> &) override;
		bool canCraft(const std::shared_ptr<Container> &input_container) override;
		/** Crafting will fail if there's not enough space in the output inventory. */
		bool craft(const std::shared_ptr<Game> &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) override;
		void toJSON(nlohmann::json &) const override;

		static CraftingRecipe fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &);
	};

	void to_json(nlohmann::json &, const CraftingRecipe &);

	struct CraftingRecipeRegistry: UnnamedJSONRegistry<CraftingRecipe> {
		static Identifier ID() { return {"base", "registry/crafting_recipe"}; }
		CraftingRecipeRegistry(): UnnamedJSONRegistry(ID()) {}
	};
}
