#include <cmath>

#include "entity/Merchant.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "game/Stonks.h"

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

	bool totalSellPrice(const Merchant &merchant, const ItemStack &stack, size_t &out) {
		const auto &inventory = *merchant.inventory;
		auto merchant_amount = inventory.count(stack);
		const auto base = stack.item->basePrice;
		double price = 0.;
		MoneyCount merchant_money = merchant.money;
		const double greed = merchant.greed;
		bool result = true;
		auto amount = stack.count;
		while (1 <= amount) {
			const double unit_price = sellPrice(base, merchant_amount++, merchant_money, greed);
			if (merchant_money < unit_price)
				result = false;
			merchant_money -= unit_price;
			price += unit_price;
			--amount;
		}

		if (0 < amount) {
			const double subunit_price = amount * sellPrice(base, merchant_amount, merchant_money, greed);
			if (merchant_money < subunit_price)
				result = false;
			merchant_money -= subunit_price;
			price += subunit_price;
		}

		const size_t discrete_price = std::floor(price);
		out = discrete_price;
		return result? discrete_price <= merchant.money : false;
	}

	size_t totalBuyPrice(const Merchant &merchant, const ItemStack &stack) {
		double merchant_amount = merchant.inventory->count(stack);
		const double base = stack.item->basePrice;
		double price = 0.;
		double merchant_money = merchant.money;
		auto amount = stack.count;
		while (1 <= amount) {
			const double unit_price = buyPrice(base, merchant_amount--, merchant_money);
			merchant_money += unit_price;
			price += unit_price;
			--amount;
		}

		if (0 < amount) {
			const double subunit_price = amount * buyPrice(base, merchant_amount, merchant_money);
			merchant_money += subunit_price;
			price += subunit_price;
		}

		// It's assumed the caller will check whether the player has enough money.
		return std::ceil(price);
	}
}
