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

	struct CombinerRecipe: Recipe<std::vector<ItemStackPtr>, ItemStackPtr, NamedRegisterable> {
		ItemCount outputCount{};
		Input input;
		Identifier outputID;

		CombinerRecipe() = default;
		CombinerRecipe(Identifier);
		CombinerRecipe(Identifier, const std::shared_ptr<Game> &, const nlohmann::json &);

		Input getInput(const std::shared_ptr<Game> &) override;
		Output getOutput(const Input &, const std::shared_ptr<Game> &) override;
		/** Doesn't lock the container. */
		bool canCraft(const std::shared_ptr<Container> &) override;
		/** Doesn't lock either container. */
		bool craft(const std::shared_ptr<Game> &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftover) override;
		/** Doesn't lock either container. Computationally expensive. */
		bool craft(const std::shared_ptr<Game> &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) override;
		void toJSON(nlohmann::json &) const override;
	};

	void to_json(nlohmann::json &, const CombinerRecipe &);

	struct CombinerRecipeRegistry: NamedRegistry<CombinerRecipe> {
		static Identifier ID() { return {"base", "registry/combiner"}; }
		CombinerRecipeRegistry(): NamedRegistry(ID()) {}
	};
}
