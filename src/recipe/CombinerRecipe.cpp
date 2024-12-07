#include "game/Game.h"
#include "game/Inventory.h"
#include "recipe/CombinerRecipe.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"

namespace Game3 {
	CombinerRecipe::CombinerRecipe(Identifier output_):
		Recipe(output_), outputID(std::move(output_)) {}

	CombinerRecipe::CombinerRecipe(Identifier output_, const std::shared_ptr<Game> &game, const boost::json::value &json):
		Recipe(output_), input(boost::json::value_to<CombinerInput>(json, &outputCount).getStacks(game)), outputID(std::move(output_)) {}

	CombinerRecipe::Input CombinerRecipe::getInput(const std::shared_ptr<Game> &) {
		return input;
	}

	CombinerRecipe::Output CombinerRecipe::getOutput(const Input &, const std::shared_ptr<Game> &game) {
		return ItemStack::create(game, outputID, outputCount);
	}

	bool CombinerRecipe::canCraft(const std::shared_ptr<Container> &container) {
		if (auto inventory = std::dynamic_pointer_cast<Inventory>(container)) {
			// Assumption: the same type of item won't be listed multiple times in one CombinerInput::inputs.
			for (const ItemStackPtr &stack: input)
				if (!inventory->contains(stack))
					return false;
		}

		return true;
	}

	bool CombinerRecipe::craft(const std::shared_ptr<Game> &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftover) {
		auto input_inventory  = std::dynamic_pointer_cast<Inventory>(input_container);
		auto output_inventory = std::dynamic_pointer_cast<Inventory>(output_container);

		if (!input_inventory || !output_inventory || !canCraft(input_inventory))
			return false;

		Output output = getOutput(input, game);

		ItemStackPtr output_stack = ItemStack::create(game, identifier, outputCount);

		if (!output_inventory->canInsert(output_stack))
			return false;

		for (const ItemStackPtr &input_stack: input) {
			const bool matches = input_stack->count == input_inventory->remove(input_stack);
			assert(matches);
		}

		if (ItemStackPtr insertion_leftover = output_inventory->add(output_stack))
			leftover = std::move(insertion_leftover);
		else
			leftover.reset();

		return true;
	}

	bool CombinerRecipe::craft(const std::shared_ptr<Game> &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) {
		auto input_inventory  = std::dynamic_pointer_cast<Inventory>(input_container);
		auto output_inventory = std::dynamic_pointer_cast<Inventory>(output_container);

		if (!input_inventory || !output_inventory || !canCraft(input_inventory))
			return false;

		Output output = getOutput(input, game);

		ItemStackPtr output_stack = ItemStack::create(game, identifier, outputCount);

		if (!output_inventory->canInsert(output_stack))
			return false;

		for (const ItemStackPtr &input_stack: input) {
			const bool matches = input_stack->count == input_inventory->remove(input_stack);
			assert(matches);
		}

		auto copy = output_inventory->copy();

		if (ItemStackPtr insertion_leftover = copy->add(output_stack))
			return false;

		output_inventory->replace(std::move(*copy));
		return true;
	}

	void CombinerRecipe::toJSON(boost::json::value &json) const {
		auto &object = json.emplace_object();
		object["type"] = boost::json::value_from(CombinerRecipeRegistry::ID());
		object["input"] = boost::json::value_from(input);
		object["output"] = {outputID, outputCount};
	}
}
