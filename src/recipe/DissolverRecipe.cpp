#include "game/Game.h"
#include "game/Inventory.h"
#include "lib/JSON.h"
#include "recipe/DissolverRecipe.h"
#include "threading/ThreadContext.h"
#include "util/Util.h"


#include "chemskr/Chemskr.h"

namespace Game3 {
	namespace {
		size_t countAtoms(const ItemStackPtr &chemical) {
			if (chemical->item->identifier != "base:item/chemical") {
				return 4; // Count non-chemicals as four atoms.
			}

			const auto counts = Chemskr::count(std::string(chemical->data.at("formula").as_string()));

			return std::accumulate(counts.begin(), counts.end(), 0, [](size_t total, const auto &pair) {
				return total + pair.second;
			});
		}
	}

	DissolverRecipe::DissolverRecipe(Identifier identifier_):
		Recipe(std::move(identifier_)) {}

	DissolverRecipe::DissolverRecipe(Identifier identifier_, Input input_, const boost::json::value &json):
		Recipe(std::move(identifier_)), input(std::move(input_)), dissolverResult(DissolverResult::fromJSON(json)) {}

	DissolverRecipe::Input DissolverRecipe::getInput(const GamePtr &) {
		return input;
	}

	DissolverRecipe::Output DissolverRecipe::getOutput(const Input &, const GamePtr &game) {
		return dissolverResult->getResult(game);
	}

	bool DissolverRecipe::canCraft(const std::shared_ptr<Container> &container) {
		if (auto inventory = std::dynamic_pointer_cast<Inventory>(container))
			return inventory->contains(input);

		return false;
	}

	bool DissolverRecipe::craft(const GamePtr &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers, size_t *atoms_out) {
		auto input_inventory  = std::dynamic_pointer_cast<Inventory>(input_container);
		auto output_inventory = std::dynamic_pointer_cast<Inventory>(output_container);

		if (!input_inventory || !output_inventory || !canCraft(input_inventory))
			return false;

		Output output = getOutput(input, game);

		size_t atom_count = 0;

		for (const ItemStackPtr &stack: output) {
			if (!output_inventory->canInsert(stack))
				return false;
			atom_count += countAtoms(stack);
		}

		assert(input->count == input_inventory->remove(input));

		for (const ItemStackPtr &stack: output) {
			if (ItemStackPtr leftover_stack = output_inventory->add(stack)) {
				if (!leftovers)
					leftovers.emplace();
				leftovers->push_back(std::move(leftover_stack));
			}
		}

		if (atoms_out)
			*atoms_out = atom_count;

		return true;
	}

	bool DissolverRecipe::craft(const GamePtr &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container, std::optional<Output> &leftovers) {
		return craft(game, input_container, output_container, leftovers, nullptr);
	}

	bool DissolverRecipe::craft(const GamePtr &game, const std::shared_ptr<Container> &input_container, const std::shared_ptr<Container> &output_container) {
		auto input_inventory  = std::dynamic_pointer_cast<Inventory>(input_container);
		auto output_inventory = std::dynamic_pointer_cast<Inventory>(output_container);

		if (!input_inventory || !output_inventory || !canCraft(input_inventory))
			return false;

		Output output = getOutput(input, game);

		auto copy = output_inventory->copy();

		assert(input->count == input_inventory->remove(input));

		for (const ItemStackPtr &stack: output)
			if (ItemStackPtr leftover_stack = output_inventory->add(stack))
				return false;

		output_inventory->replace(std::move(*copy));
		return true;
	}

	DissolverRecipe DissolverRecipe::fromJSON(const GamePtr &game, const Identifier &identifier, const boost::json::value &json) {
		return DissolverRecipe(identifier, ItemStack::create(game, identifier, 1), json);
	}

	void DissolverRecipe::toJSON(boost::json::value &json, const GamePtr &) const {
		assert(dissolverResult);
		auto &object = json.emplace_object();
		object["type"] = boost::json::value_from(DissolverRecipeRegistry::ID());
		object["input"] = boost::json::value_from(*input);
		object["output"] = boost::json::value_from(*dissolverResult);
	}

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, const DissolverRecipe &recipe, const GamePtr &game) {
		recipe.toJSON(json, game);
	}
}
