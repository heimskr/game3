#include "game/Item.h"

namespace Game3 {
	std::map<ItemID, std::shared_ptr<Item>> Item::ITEMS {
		{Item::SHORTSWORD, std::make_shared<Item>(Item::SHORTSWORD, "Shortsword", 1)},
		{Item::RED_POTION, std::make_shared<Item>(Item::RED_POTION, "Red Potion", 8)},
	};

	bool ItemStack::canMerge(const ItemStack &other) const {
		// To be updated when items can store data.
		return item->id == other.item->id;
	}

	void to_json(nlohmann::json &json, const ItemStack &stack) {
		json[0] = stack.item->id;
		json[1] = stack.count;
	}

	void from_json(const nlohmann::json &json, ItemStack &stack) {
		stack.item = Item::ITEMS.at(json.at(0));
		stack.count = json.at(1);
	}
}
