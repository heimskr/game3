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
				if (0 < stack.count && inventory->count(stack) < stack.count)
					return false;
			} else {
				const auto &[attribute, count] = requirement.get<AttributeRequirement>();
				if (0 < count && inventory->countAttribute(attribute) < count)
					return false;
			}
		}

		return true;
	}

	bool CraftingRecipe::craft(Game &, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) {
		auto inventory_in  = std::dynamic_pointer_cast<Inventory>(input_container);
		auto inventory_out = std::dynamic_pointer_cast<Inventory>(output_container);

		if (!inventory_in || !inventory_out || !canCraft(input_container))
			return false;

		leftovers.emplace();

		for (const CraftingRequirement &requirement: input)
			inventory_in->remove(requirement);

		for (const ItemStack &stack: output)
			if (std::optional<ItemStack> leftover = inventory_out->add(stack))
				leftovers->push_back(std::move(*leftover));

		return true;
	}

	CraftingRecipe CraftingRecipe::fromJSON(const Game &game, const nlohmann::json &json) {
		CraftingRecipe recipe;

		for (const auto &item: json.at("input"))
			recipe.input.emplace_back(CraftingRequirement::fromJSON(game, item));

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
