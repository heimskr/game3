#pragma once

#include "types/Types.h"
#include "item/Item.h"

#include <memory>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Game;
	class ItemStack;

	class Product {
		protected:
			Product() = default;

		public:
			virtual ~Product() = default;
			virtual std::vector<ItemStackPtr> sample() const = 0;
	};

	/** Yields an ItemStack with a constant count. */
	class ConstantProduct: public Product {
		private:
			ItemStackPtr stack;
			ItemCount count;

		public:
			ConstantProduct(ItemStackPtr, ItemCount);
			std::vector<ItemStackPtr> sample() const override;
	};

	/** Yields an ItemStack with a count that starts at a number and increases as long as a certain random chance is met. */
	class ExponentialProduct: public Product {
		private:
			ItemStackPtr stack;
			ItemCount start;
			double chance;

		public:
			ExponentialProduct(ItemStackPtr, ItemCount start_, double chance_);
			std::vector<ItemStackPtr> sample() const override;
	};

	class Products {
		public:
			Products() = default;

			std::vector<ItemStackPtr> getStacks() const;

			static Products fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &);

		private:
			std::vector<std::unique_ptr<Product>> products;

	};
}
