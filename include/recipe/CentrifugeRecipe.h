#pragma once

#include "data/Identifier.h"
#include "game/Fluids.h"
#include "item/Item.h"
#include "recipe/Recipe.h"
#include "registry/Registries.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	struct CentrifugeRecipe: Recipe<FluidStack, ItemStack> {
		Input input;
		std::map<nlohmann::json, double> weightMap;

		CentrifugeRecipe() = default;
		CentrifugeRecipe(Input, std::map<nlohmann::json, double>);

		Input getInput(Game &) override;
		Output getOutput(const Input &, Game &) override;
		bool canCraft(const std::shared_ptr<Container> &) override;
		bool craft(Game &, const std::shared_ptr<Container> &, std::optional<Output> &leftovers) override;

		static CentrifugeRecipe fromJSON(const Game &, const nlohmann::json &);
	};

	// void from_json(const nlohmann::json &, CentrifugeRecipe &);
	void to_json(nlohmann::json &, const CentrifugeRecipe &);

	struct CentrifugeRecipeRegistry: UnnamedJSONRegistry<CentrifugeRecipe> {
		static Identifier ID() { return {"base", "centrifuge_recipe"}; }
		CentrifugeRecipeRegistry(): UnnamedJSONRegistry(ID()) {}
	};
}
