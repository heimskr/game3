#include "item/Products.h"
#include "lib/JSON.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	ConstantProduct::ConstantProduct(ItemStackPtr stack_, ItemCount count_):
		stack(std::move(stack_)), count(count_) {}

	std::vector<ItemStackPtr> ConstantProduct::sample() const {
		return {stack->withCount(count)};
	}

	ExponentialProduct::ExponentialProduct(ItemStackPtr stack_, ItemCount start_, double chance_):
		stack(std::move(stack_)), start(start_), chance(chance_) {}

	std::vector<ItemStackPtr> ExponentialProduct::sample() const {
		ItemCount count = start;

		while (std::uniform_real_distribution(0., 1.)(threadContext.rng) <= chance) {
			++count;
		}

		return {stack->withCount(count)};
	}

	std::vector<ItemStackPtr> Products::getStacks() const {
		std::vector<ItemStackPtr> out;
		out.reserve(products.size());
		for (const auto &product: products) {
			for (const ItemStackPtr &stack: product->sample()) {
				out.push_back(stack);
			}
		}
		return out;
	}

	Products tag_invoke(boost::json::value_to_tag<Products>, const boost::json::value &json, const std::shared_ptr<Game> &game) {
		Products out;

		for (const boost::json::value &item: json.as_array()) {
			std::string type(item.at(0).as_string());
			if (type == "constant") {
				out.products.emplace_back(std::make_unique<ConstantProduct>(boost::json::value_to<ItemStackPtr>(item.at(1), game), boost::json::value_to<ItemCount>(item.at(2))));
			} else if (type == "exponential") {
				out.products.emplace_back(std::make_unique<ExponentialProduct>(boost::json::value_to<ItemStackPtr>(item.at(1), game), boost::json::value_to<ItemCount>(item.at(2)), getDouble(item.at(3))));
			} else {
				throw std::invalid_argument(std::format("Invalid product type: \"{}\"", type));
			}
		}

		return out;
	}
}
