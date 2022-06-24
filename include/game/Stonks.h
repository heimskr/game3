#pragma once

#include <string>

#include "Types.h"

/** Base prices go through multiple stages of modification before they're used by merchants. This namespace contains
 *  functions to transform prices through various steps of modification. A buy price is the amount of money the player
 *  has to pay for a resource, while the sell price is how much money the player can receive in exchange for it. */
namespace Game3 {
	class ItemStack;
	class Keep;
	class Merchant;

	constexpr double E = 2.71828182845904523536;

	/** Returns the price for which a merchant will buy a resource, given the resource's sell price. */
	double buyPriceToSellPrice(double buy_price, double greed);

	/** Prices depend on the amount of money a merchant has. This adjusts a price accordingly. */
	double applyMoney(double base_price, size_t merchant_money);

	/** Prices depend on the amount of the resource a merchant owns. This adjusts a price accordingly. */
	double applyScarcity(double base_price, ItemCount item_count);

	/** Determines the buy price for a single unit of a resource. */
	double buyPrice(double base_price, ItemCount item_count, size_t merchant_money);

	/** Determines the sell price for a single unit of a resource. */
	double sellPrice(double base_price, ItemCount item_count, size_t merchant_money, double greed);

	/** Determines whether the merchant can afford a given amount of a certain resource, and if so, outputs the price.
	 *  Unfortunately, this is O(amount). */
	bool totalSellPrice(const Inventory &, MoneyCount, double greed, const ItemStack &, MoneyCount &out);

	/** Determines whether the merchant can afford a given amount of a certain resource, and if so, outputs the price.
	 *  Unfortunately, this is O(amount). */
	bool totalSellPrice(const Merchant &, const ItemStack &, MoneyCount &out);

	/** Determines whether the keep can afford a given amount of a certain resource, and if so, outputs the price.
	 *  Unfortunately, this is O(amount). */
	bool totalSellPrice(const Keep &, const ItemStack &, MoneyCount &out);

	/** Returns the price to buy a given amount of a resource from a merchant. Like totalSellPrice, this is O(amount).
	 *  The caller should check whether the player has enough money. */
	size_t totalBuyPrice(const Inventory &, MoneyCount, const ItemStack &);

	/** Returns the price to buy a given amount of a resource from a merchant. Like totalSellPrice, this is O(amount).
	 *  The caller should check whether the player has enough money. */
	size_t totalBuyPrice(const Merchant &, const ItemStack &);

	/** Returns the price to buy a given amount of a resource from a merchant. Like totalSellPrice, this is O(amount).
	 *  The caller should check whether the player has enough money. */
	size_t totalBuyPrice(const Keep &, const ItemStack &);
}
