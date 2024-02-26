#include "item/Products.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	ConstantProduct::ConstantProduct(ItemStack stack_, ItemCount count_):
		stack(std::move(stack_)), count(count_) {}

	std::vector<ItemStack> ConstantProduct::sample() const {
		return {stack.withCount(count)};
	}

	ExponentialProduct::ExponentialProduct(ItemStack stack_, ItemCount start_, double chance_):
		stack(std::move(stack_)), start(start_), chance(chance_) {}

	std::vector<ItemStack> ExponentialProduct::sample() const {
		ItemCount count = start;

		while (std::uniform_real_distribution(0., 1.)(threadContext.rng) <= chance)
			++count;

		return {stack.withCount(count)};
	}

	std::vector<ItemStack> Products::getStacks() const {
		std::vector<ItemStack> out;
		out.reserve(products.size());
		for (const auto &product: products)
			for (ItemStack &stack: product->sample())
				out.push_back(std::move(stack));
		return out;
	}

	Products Products::fromJSON(const std::shared_ptr<Game> &game, const nlohmann::json &json) {
		Products out;

		for (const nlohmann::json &item: json) {
			const std::string type = item.at(0);
			if (type == "constant")
				out.products.emplace_back(std::make_unique<ConstantProduct>(ItemStack::fromJSON(game, item.at(1)), item.at(2)));
			else if (type == "exponential")
				out.products.emplace_back(std::make_unique<ExponentialProduct>(ItemStack::fromJSON(game, item.at(1)), item.at(2), item.at(3)));
			else
				throw std::invalid_argument("Invalid product type: \"" + type + '"');
		}

		return out;
	}
}
