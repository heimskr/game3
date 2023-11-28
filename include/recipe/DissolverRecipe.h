#pragma once

#include "chemistry/DissolverResults.h"
#include "data/Identifier.h"
#include "item/Item.h"
#include "recipe/Recipe.h"
#include "registry/Registries.h"

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class DissolverResult;

	struct DissolverRecipe: Recipe<ItemStack, std::vector<ItemStack>, NamedRegisterable> {
		Input input;
		std::unique_ptr<DissolverResult> dissolverResult;

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

		void toJSON(nlohmann::json &) const override;

		static DissolverRecipe fromJSON(const Game &, const Identifier &, const nlohmann::json &);
	};

	void to_json(nlohmann::json &, const DissolverRecipe &);

	struct DissolverRecipeRegistry: NamedRegistry<DissolverRecipe> {
		static Identifier ID() { return {"base", "registry/dissolver"}; }
		DissolverRecipeRegistry(): NamedRegistry(ID()) {}
	};
}

