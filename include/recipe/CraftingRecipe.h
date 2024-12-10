#pragma once

#include "data/Identifier.h"
#include "item/Item.h"
#include "recipe/CraftingRequirement.h"
#include "recipe/Recipe.h"
#include "registry/Registries.h"

#include <boost/json/fwd.hpp>

namespace Game3 {
	struct CraftingRecipe: Recipe<std::vector<CraftingRequirement>, std::vector<ItemStackPtr>> {
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
		/** Crafting will fail if there's not enough space in the output inventory. */
		bool craft(const std::shared_ptr<Game> &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) override;
		void toJSON(boost::json::value &, const GamePtr &) const override;
	};

	using CraftingRecipePtr = std::shared_ptr<CraftingRecipe>;

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const CraftingRecipe &);
	CraftingRecipe tag_invoke(boost::json::value_to_tag<CraftingRecipe>, const boost::json::value &, const GamePtr &);

	struct CraftingRecipeRegistry: UnnamedJSONRegistry<CraftingRecipe> {
		static Identifier ID() { return {"base", "registry/crafting_recipe"}; }
		CraftingRecipeRegistry(): UnnamedJSONRegistry(ID()) {}
	};
}
