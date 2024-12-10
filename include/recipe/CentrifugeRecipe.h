#pragma once

#include "data/Identifier.h"
#include "game/Fluids.h"
#include "item/Item.h"
#include "recipe/Recipe.h"
#include "registry/Registries.h"

#include <memory>
#include <optional>
#include <unordered_map>

namespace Game3 {
	struct CentrifugeRecipe: Recipe<FluidStack, ItemStackPtr> {
		Input input;
		std::unordered_map<boost::json::value, double> weightMap;

		CentrifugeRecipe() = default;
		CentrifugeRecipe(Input, std::unordered_map<boost::json::value, double>);

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

	CentrifugeRecipe tag_invoke(boost::json::value_to_tag<CentrifugeRecipe>, const boost::json::value &, const GamePtr &);

	struct CentrifugeRecipeRegistry: UnnamedJSONRegistry<CentrifugeRecipe> {
		static Identifier ID() { return {"base", "registry/centrifuge_recipe"}; }
		CentrifugeRecipeRegistry(): UnnamedJSONRegistry(ID()) {}
	};
}
