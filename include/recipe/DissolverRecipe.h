#pragma once

#include "chemistry/ChemistryResults.h"
#include "data/Identifier.h"
#include "item/Item.h"
#include "recipe/Recipe.h"
#include "registry/Registries.h"

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class ChemistryResult;

	struct DissolverRecipe: Recipe<ItemStack, std::vector<ItemStack>, NamedRegisterable> {
		Input input;
		std::unique_ptr<ChemistryResult> chemistryResult;

		DissolverRecipe(Identifier);
		DissolverRecipe(Identifier, Input, const nlohmann::json &);

		Input getInput(Game &) override;
		Output getOutput(const Input &, Game &) override;
		/** Doesn't lock the container. */
		bool canCraft(const std::shared_ptr<Container> &) override;
		/** Doesn't lock either container. */
		bool craft(Game &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers, size_t *atoms_out);
		/** Doesn't lock either container. */
		bool craft(Game &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) override;

		static DissolverRecipe fromJSON(const Game &, const Identifier &, const nlohmann::json &);
	};

	struct DissolverRecipeRegistry: NamedRegistry<DissolverRecipe> {
		static Identifier ID() { return {"base", "registry/dissolver"}; }
		DissolverRecipeRegistry(): NamedRegistry(ID()) {}
	};
}

