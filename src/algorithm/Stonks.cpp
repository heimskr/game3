#include "algorithm/Stonks.h"
#include "entity/Merchant.h"
#include "game/Game.h"
#include "game/Inventory.h"
#include "item/Item.h"

#include <cmath>

namespace Game3 {
	namespace {
		constexpr double E = 2.71828182845904523536;
	}

	bool isSellable(const ItemStackPtr &stack) {
		return stack->data.is_null();
	}

	double buyPriceToSellPrice(double buy_price, double greed) {
		return buy_price / (1. + greed);
	}

	double applyMoney(double base_price, MoneyCount merchant_money) {
		if (merchant_money == MoneyCount(-1)) {
			return base_price;
		}
		return base_price / pow(E, merchant_money / 50.);
	}

	double applyScarcity(double base_price, ItemCount item_count) {
		return base_price / pow(E, item_count / 100.);
	}

	double buyPrice(double base_price, ItemCount item_count, MoneyCount merchant_money) {
		return applyScarcity(applyMoney(base_price, merchant_money), item_count);
	}

	double sellPrice(double base_price, ItemCount item_count, MoneyCount merchant_money, double greed) {
		return buyPriceToSellPrice(buyPrice(base_price, item_count, merchant_money), greed);
	}

	bool totalSellPrice(const Inventory &inventory, MoneyCount money, double greed, const ItemStackPtr &stack, MoneyCount &out) {
		auto held_amount = inventory.count(stack);
		const auto base = stack->item->basePrice;
		double price = 0.;
		bool result = true;
		auto amount = stack->count;
		while (1 <= amount) {
			const double unit_price = sellPrice(base, held_amount++, money, greed);
			if (money < unit_price) {
				result = false;
			}
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

	bool totalSellPrice(const Merchant &merchant, const ItemStackPtr &stack, MoneyCount &out) {
		return totalSellPrice(*merchant.getInventory(0), merchant.getMoney(), merchant.greed, stack, out);
	}

	std::optional<MoneyCount> totalSellPrice(ItemCount merchant_count, MoneyCount merchant_money, double base_price, ItemCount count, double greed) {
		double price = 0.;
		double money(merchant_money);

		if (merchant_money == static_cast<MoneyCount>(-1)) {
			money = -1;
		}

		while (1 <= count) {
			const double unit_price = sellPrice(base_price, merchant_count++, money < 0? MoneyCount(-1) : MoneyCount(money), greed);

			if (0 <= money) {
				money -= unit_price;
				if (money < 0) {
					return std::nullopt;
				}
			}

			price += unit_price;
			--count;
		}

		return MoneyCount(std::floor(price));
	}

	std::optional<MoneyCount> totalBuyPrice(ItemCount merchant_count, MoneyCount merchant_money, double base_price, ItemCount count) {
		double price = 0.;
		double money(merchant_money);

		while (1 <= count) {
			if (merchant_count == 0) {
				return std::nullopt;
			}

			const double unit_price = buyPrice(base_price, merchant_count--, money < 0? MoneyCount(-1) : MoneyCount(money));

			if (0 <= money) {
				money += unit_price;
			}

			price += unit_price;
			--count;
		}

		return std::ceil(price);
	}

	std::optional<MoneyCount> totalBuyPrice(const Inventory &inventory, MoneyCount merchant_money, const ItemStackPtr &stack) {
		return totalBuyPrice(inventory.count(stack), merchant_money, stack->item->basePrice, stack->count);
	}

	std::optional<MoneyCount> totalBuyPrice(const Merchant &merchant, const ItemStackPtr &stack) {
		return totalBuyPrice(*merchant.getInventory(0), merchant.getMoney(), stack);
	}
}
