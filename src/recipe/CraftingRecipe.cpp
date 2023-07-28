#include "game/Inventory.h"
#include "recipe/CraftingRecipe.h"

namespace Game3 {
	CraftingRecipe::CraftingRecipe(Input input_, Output output_, Identifier station_type):
		input(std::move(input_)), output(std::move(output_)), stationType(std::move(station_type)) {}

	CraftingRecipe::Input CraftingRecipe::getInput(Game &) {
		return input;
	}

	CraftingRecipe::Output CraftingRecipe::getOutput(const Input &, Game &) {
		return output;
	}

	bool CraftingRecipe::canCraft(const std::shared_ptr<Container> &container) {
		auto inventory = std::dynamic_pointer_cast<Inventory>(container);
		if (!inventory)
			return false;

		for (const auto &requirement: input) {
			if (requirement.is<ItemStack>()) {
				const auto &stack = requirement.get<ItemStack>();
				if (inventory->count(stack) < stack.count)
					return false;
			} else {
				const auto &[attribute, count] = requirement.get<AttributeRequirement>();
				if (inventory->countAttribute(attribute) < count)
					return false;
			}
		}

		return true;
	}

	bool CraftingRecipe::craft(Game &, const std::shared_ptr<Container> &container, std::optional<Output> &leftovers) {
		auto inventory = std::dynamic_pointer_cast<Inventory>(container);

		if (!inventory || !canCraft(container))
			return false;

		leftovers.emplace();

		for (const auto &requirement: input)
			inventory->remove(requirement);

		for (const auto &stack: output)
			if (auto leftover = inventory->add(stack))
				leftovers->push_back(std::move(*leftover));

		return true;
	}

	CraftingRecipe CraftingRecipe::fromJSON(const Game &game, const nlohmann::json &json) {
		CraftingRecipe recipe;

		for (const auto &item: json.at("input"))
			recipe.input.push_back(CraftingRequirement::fromJSON(game, item));

		recipe.output = ItemStack::manyFromJSON(game, json.at("output"));

		if (auto iter = json.find("station"); iter != json.end())
			recipe.stationType = *iter;

		return recipe;
	}

	void to_json(nlohmann::json &json, const CraftingRecipe &recipe) {
		json["input"] = recipe.input;
		json["output"] = recipe.output;
		if (recipe.stationType)
			json["station"] = recipe.stationType;
	}
}
