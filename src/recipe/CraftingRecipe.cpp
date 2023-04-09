#include "game/Inventory.h"
#include "recipe/CraftingRecipe.h"

namespace Game3 {
	CraftingRecipe::CraftingRecipe(Input input_, Output output_, Identifier station_type):
		input(std::move(input_)), output(std::move(output_)), stationType(std::move(station_type)) {}

	CraftingRecipe::Input CraftingRecipe::getInput() {
		return input;
	}

	CraftingRecipe::Output CraftingRecipe::getOutput() {
		return output;
	}

	bool CraftingRecipe::canCraft(const std::shared_ptr<Container> &container) {
		auto inventory = std::dynamic_pointer_cast<Inventory>(container);
		if (!inventory)
			return false;

		for (const auto &stack: input)
			if (inventory->count(stack) < stack.count)
				return false;

		return true;
	}

	bool CraftingRecipe::craft(const std::shared_ptr<Container> &container, Output &leftovers) {
		auto inventory = std::dynamic_pointer_cast<Inventory>(container);

		if (!inventory || !canCraft(container))
			return false;

		leftovers.clear();

		for (const auto &stack: input)
			inventory->remove(stack);

		for (const auto &stack: output)
			if (auto leftover = inventory->add(stack))
				leftovers.push_back(std::move(*leftover));

		return true;
	}
}
