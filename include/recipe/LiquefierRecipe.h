#pragma once

#include "data/Identifier.h"
#include "game/Fluids.h"
#include "item/Item.h"
#include "recipe/Recipe.h"
#include "registry/Registries.h"

#include <boost/json/fwd.hpp>

namespace Game3 {
	struct LiquefierRecipe: Recipe<ItemStackPtr, FluidStack> {
		Input input;
		Output output;

		LiquefierRecipe() = default;
		LiquefierRecipe(Input, Output);

		Input getInput(const GamePtr &) override;
		Output getOutput(const Input &, const GamePtr &) override;
		/** Doesn't lock the container. */
		bool canCraft(const std::shared_ptr<Container> &) override;
		/** Doesn't lock either container. */
		bool craft(const GamePtr &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) override;
		/** Doesn't lock either container. */
		bool craft(const GamePtr &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) override;
		void toJSON(boost::json::value &, const GamePtr &) const override;
	};

	LiquefierRecipe tag_invoke(boost::json::value_to_tag<LiquefierRecipe>, const boost::json::value &, const GamePtr &);

	struct LiquefierRecipeRegistry: UnnamedJSONRegistry<LiquefierRecipe> {
		static Identifier ID() { return {"base", "registry/liquefier_recipe"}; }
		LiquefierRecipeRegistry(): UnnamedJSONRegistry(ID()) {}
	};
}
