#include "Log.h"
#include "graphics/Tileset.h"
#include "game/ClientGame.h"
#include "packet/RecipeListPacket.h"
#include "recipe/CraftingRecipe.h"

namespace Game3 {
	void RecipeListPacket::handle(const ClientGamePtr &game) {
		auto &registry = game->registry<CraftingRecipeRegistry>();
		auto lock = registry.uniqueLock();
		registry.clear();
		for (const auto &json: recipes) {
			registry.add(game, json);
		}
	}

	std::vector<boost::json::value> RecipeListPacket::getRecipes(const CraftingRecipeRegistry &registry, const GamePtr &game) {
		std::vector<boost::json::value> out;
		auto lock = registry.sharedLock();
		for (const auto &recipe: registry) {
			boost::json::value recipe_json;
			recipe->toJSON(recipe_json, game);
			out.emplace_back(std::move(recipe_json));
			auto *object = out.back().if_object();
			assert(object != nullptr);
			object->erase("type");
		}
		return out;
	}
}
