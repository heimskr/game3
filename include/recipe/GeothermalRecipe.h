#pragma once

#include "data/Identifier.h"
#include "game/Fluids.h"
#include "item/Item.h"
#include "recipe/Recipe.h"
#include "registry/Registries.h"

namespace Game3 {
	struct GeothermalRecipe: Recipe<FluidStack, EnergyAmount> {
		Input input;
		Output output;

		GeothermalRecipe() = default;
		GeothermalRecipe(Input, Output);

		Input getInput(const std::shared_ptr<Game> &) override;
		Output getOutput(const Input &, const std::shared_ptr<Game> &) override;
		/** Doesn't lock the container. */
		bool canCraft(const std::shared_ptr<Container> &) override;
		/** Doesn't lock either container. Fails if there would be leftover energy. */
		bool craft(const std::shared_ptr<Game> &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) override;
		/** Doesn't lock either container. */
		bool craft(const std::shared_ptr<Game> &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) override;
		void toJSON(boost::json::value &) const override;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const GeothermalRecipe &);
	GeothermalRecipe tag_invoke(boost::json::value_to_tag<GeothermalRecipe>, const boost::json::value &, const std::shared_ptr<Game> &);

	class GeothermalRecipeRegistry: public UnnamedJSONRegistry<GeothermalRecipe> {
		public:
			std::unordered_set<FluidID> fluidIDs;

			static Identifier ID() { return {"base", "registry/geothermal_recipe"}; }

			GeothermalRecipeRegistry(): UnnamedJSONRegistry(ID()) {}

			void onAdd(const GeothermalRecipe &recipe) override {
				fluidIDs.insert(recipe.input.id);
			}
	};
}
