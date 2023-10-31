#pragma once

#include "chemistry/CombinerInput.h"
#include "data/Identifier.h"
#include "item/Item.h"
#include "recipe/Recipe.h"
#include "registry/Registries.h"

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class CombinerResult;
	class Game;

	struct CombinerRecipe: Recipe<std::vector<ItemStack>, ItemStack, NamedRegisterable> {
		ItemCount outputCount{};
		Input input;

		CombinerRecipe(Identifier);
		CombinerRecipe(Identifier, Game &, const nlohmann::json &);

		Input getInput(Game &) override;
		Output getOutput(const Input &, Game &) override;
		/** Doesn't lock the container. */
		bool canCraft(const std::shared_ptr<Container> &) override;
		/** Doesn't lock either container. */
		bool craft(Game &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftover) override;
	};

	struct CombinerRecipeRegistry: NamedRegistry<CombinerRecipe> {
		static Identifier ID() { return {"base", "registry/combiner"}; }
		CombinerRecipeRegistry(): NamedRegistry(ID()) {}
	};
}
