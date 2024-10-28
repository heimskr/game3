#pragma once

#include "recipe/CraftingRecipe.h"

#include <map>
#include <vector>

namespace Game3 {
	class Player;

	struct KnownCraftingRecipes {
		std::vector<CraftingRecipePtr> partial;
		std::vector<CraftingRecipePtr> full;

		KnownCraftingRecipes() = default;
		KnownCraftingRecipes(std::vector<CraftingRecipePtr> partial, std::vector<CraftingRecipePtr> full);
	};

	class CraftingManager {
		public:
			CraftingManager(Player &);

			KnownCraftingRecipes getKnownRecipes();

		private:
			Player &player;
			std::optional<std::multimap<Identifier, CraftingRecipePtr>> craftingRecipeIndex;

			std::multimap<Identifier, CraftingRecipePtr> generateCraftingRecipeIndex() const;
	};
}
