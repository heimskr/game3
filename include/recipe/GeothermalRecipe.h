#pragma once

#include "data/Identifier.h"
#include "game/Fluids.h"
#include "item/Item.h"
#include "recipe/Recipe.h"
#include "registry/Registries.h"

#include <nlohmann/json.hpp>

namespace Game3 {
	struct GeothermalRecipe: Recipe<FluidStack, EnergyAmount> {
		Input input;
		Output output;

		GeothermalRecipe() = default;
		GeothermalRecipe(Input, Output);

		Input getInput(Game &) override;
		Output getOutput(const Input &, Game &) override;
		/** Doesn't lock the container. */
		bool canCraft(const std::shared_ptr<Container> &) override;
		/** Doesn't lock either container. */
		bool craft(Game &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) override;

		static GeothermalRecipe fromJSON(const Game &, const nlohmann::json &);
	};

	// void from_json(const nlohmann::json &, GeothermalRecipe &);
	void to_json(nlohmann::json &, const GeothermalRecipe &);

	struct GeothermalRecipeRegistry: UnnamedJSONRegistry<GeothermalRecipe> {
		static Identifier ID() { return {"base", "geothermal_recipe"}; }
		GeothermalRecipeRegistry(): UnnamedJSONRegistry(ID()) {}
	};
}
