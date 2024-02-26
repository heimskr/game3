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
			virtual std::vector<ItemStack> sample() const = 0;
	};

	/** Yields an ItemStack with a constant count. */
	class ConstantProduct: public Product {
		private:
			ItemStack stack;
			ItemCount count;

		public:
			ConstantProduct(ItemStack, ItemCount);
			std::vector<ItemStack> sample() const override;
	};

	/** Yields an ItemStack with a count that starts at a number and increases as long as a certain random chance is met. */
	class ExponentialProduct: public Product {
		private:
			ItemStack stack;
			ItemCount start;
			double chance;

		public:
			ExponentialProduct(ItemStack, ItemCount start_, double chance_);
			std::vector<ItemStack> sample() const override;
	};

	class Products {
		public:
			Products() = default;

			std::vector<ItemStack> getStacks() const;

			static Products fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &);

		private:
			std::vector<std::unique_ptr<Product>> products;

	};
}
