#include "Log.h"
#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "packet/RecipeListPacket.h"
#include "recipe/CraftingRecipe.h"

namespace Game3 {
	void RecipeListPacket::handle(ClientGame &game) {
		auto &registry = game.registry<CraftingRecipeRegistry>();
		registry.clear();
		for (const auto &json: recipes)
			registry.add(game, json);
	}

	std::vector<nlohmann::json> RecipeListPacket::getRecipes(const CraftingRecipeRegistry &registry) {
		std::vector<nlohmann::json> out;
		for (const auto &recipe: registry) {
			out.emplace_back(*recipe);
			out.back().erase("type");
		}
		return out;
	}
}
