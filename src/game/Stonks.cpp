#include <cmath>

#include "entity/Merchant.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/Stonks.h"
#include "realm/Keep.h"

namespace Game3 {
	double buyPriceToSellPrice(double buy_price, double greed) {
		return buy_price / (1. + greed);
	}

	double applyMoney(double base_price, size_t merchant_money) {
		return base_price / (1. + (1. / pow(E, -merchant_money / 50.)));
	}

	double applyScarcity(double base_price, ItemCount item_count) {
		return base_price / (1. + (1. / pow(E, -item_count / 100.)));
	}

	double buyPrice(double base_price, ItemCount item_count, size_t merchant_money) {
		return applyScarcity(applyMoney(base_price, merchant_money), item_count);
	}

	double sellPrice(double base_price, ItemCount item_count, size_t merchant_money, double greed) {
		return buyPriceToSellPrice(buyPrice(base_price, item_count, merchant_money), greed);
	}

	bool totalSellPrice(const Inventory &inventory, MoneyCount money, double greed, const ItemStack &stack, MoneyCount &out) {
		auto held_amount = inventory.count(stack);
		const auto base = stack.item->basePrice;
		double price = 0.;
		bool result = true;
		auto amount = stack.count;
		while (1 <= amount) {
			const double unit_price = sellPrice(base, held_amount++, money, greed);
			if (money < unit_price)
				result = false;
			money -= unit_price;
			price += unit_price;
			--amount;
		}

		// if (0 < amount) {
		// 	const double subunit_price = amount * sellPrice(base, held_amount, money, greed);
		// 	if (money < subunit_price)
		// 		result = false;
		// 	money -= subunit_price;
		// 	price += subunit_price;
		// }

		const MoneyCount discrete_price = std::floor(price);
		out = discrete_price;
		return result? discrete_price <= money : false;
	}

	bool totalSellPrice(const Merchant &merchant, const ItemStack &stack, MoneyCount &out) {
		return totalSellPrice(*merchant.inventory, merchant.money, merchant.greed, stack, out);
	}

	bool totalSellPrice(const Keep &keep, const ItemStack &stack, MoneyCount &out) {
		return totalSellPrice(*keep.stockpileInventory, keep.money, keep.greed, stack, out);
	}

	size_t totalBuyPrice(const Inventory &inventory, MoneyCount money, const ItemStack &stack) {
		double merchant_amount = inventory.count(stack);
		const double base = stack.item->basePrice;
		double price = 0.;
		auto amount = stack.count;
		while (1 <= amount) {
			const double unit_price = buyPrice(base, merchant_amount--, money);
			money += unit_price;
			price += unit_price;
			--amount;
		}

		// if (0 < amount) {
		// 	const double subunit_price = amount * buyPrice(base, merchant_amount, money);
		// 	money += subunit_price;
		// 	price += subunit_price;
		// }

		// It's assumed the caller will check whether the player has enough money.
		return std::ceil(price);
	}

	size_t totalBuyPrice(const Merchant &merchant, const ItemStack &stack) {
		return totalBuyPrice(*merchant.inventory, merchant.money, stack);
	}

	size_t totalBuyPrice(const Keep &keep, const ItemStack &stack) {
		return totalBuyPrice(*keep.stockpileInventory, keep.money, stack);
	}
}
