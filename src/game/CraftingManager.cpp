#include "entity/Player.h"
#include "game/CraftingManager.h"
#include "game/Game.h"
#include "util/Timer.h"

namespace Game3 {
	KnownCraftingRecipes::KnownCraftingRecipes(std::vector<CraftingRecipePtr> partial, std::vector<CraftingRecipePtr> full):
		partial(std::move(partial)), full(std::move(full)) {}

	CraftingManager::CraftingManager(Player &player):
		player(player) {}

	KnownCraftingRecipes CraftingManager::getKnownRecipes() {
		if (!craftingRecipeIndex) {
			craftingRecipeIndex = generateCraftingRecipeIndex();
		}

		std::set<CraftingRecipePtr> all;

		for (const auto &item_id: player.getKnownItems()) {
			auto [iter, end] = craftingRecipeIndex->equal_range(item_id);
			for (; iter != end; ++iter) {
				all.emplace(iter->second);
			}
		}

		GamePtr game = player.getGame();
		KnownCraftingRecipes out;
		auto &known_items = player.getKnownItems();
		auto lock = known_items.sharedLock();

		for (const CraftingRecipePtr &recipe: all) {
			bool was_partial = false;

			for (const CraftingRequirement &requirement: recipe->getInput(game)) {
				if (requirement.is<ItemStackPtr>()) {
					if (!known_items.contains(requirement.get<ItemStackPtr>()->getID())) {
						was_partial = true;
						break;
					}
				} else {
					const Identifier &attribute = requirement.get<AttributeRequirement>().attribute;
					if (auto iter = game->itemsByAttribute.find(attribute); iter != game->itemsByAttribute.end()) {
						bool any_known = false;
						for (const ItemPtr &item: iter->second) {
							if (known_items.contains(item->identifier)) {
								any_known = true;
								break;
							}
						}

						if (!any_known) {
							was_partial = true;
							break;
						}
					}
				}
			}

			if (was_partial) {
				out.partial.push_back(recipe);
			} else {
				out.full.push_back(recipe);
			}
		}

		auto comp = [](const CraftingRecipePtr &left, const CraftingRecipePtr &right) {
			return left->registryID < right->registryID;
		};

		std::ranges::sort(out.partial, comp);
		std::ranges::sort(out.full, comp);

		return out;
	}

	std::multimap<Identifier, CraftingRecipePtr> CraftingManager::generateCraftingRecipeIndex() const {
		Timer timer{"IndexRecipes"};
		std::multimap<Identifier, CraftingRecipePtr> out;

		GamePtr game = player.getGame();
		auto &recipe_registry = game->registry<CraftingRecipeRegistry>();
		auto &item_registry = *game->itemRegistry;

		// Maps attributes to item IDs.
		std::multimap<Identifier, Identifier> attribute_index;

		std::set<Identifier> all_attributes;

		for (const CraftingRecipePtr &recipe: recipe_registry.items) {
			for (const CraftingRequirement &requirement: recipe->input) {
				if (requirement.is<AttributeRequirement>()) {
					const AttributeRequirement &attribute_requirement = requirement.get<AttributeRequirement>();
					all_attributes.emplace(attribute_requirement.attribute);
				}
			}
		}

		for (const Identifier &attribute: all_attributes) {
			for (const auto &[item_id, item]: item_registry) {
				if (item->hasAttribute(attribute)) {
					attribute_index.emplace(attribute, item_id);
				}
			}
		}

		for (const CraftingRecipePtr &recipe: recipe_registry.items) {
			for (const CraftingRequirement &requirement: recipe->input) {
				if (requirement.is<ItemStackPtr>()) {
					const ItemStackPtr &item = requirement.get<ItemStackPtr>();
					out.emplace(item->getID(), recipe);
				} else {
					const AttributeRequirement &attribute_requirement = requirement.get<AttributeRequirement>();
					auto [iter, end] = attribute_index.equal_range(attribute_requirement.attribute);
					assert(iter != end);
					for (; iter != end; ++iter) {
						out.emplace(iter->second, recipe);
					}
				}
			}
		}

		return out;
	}
}
