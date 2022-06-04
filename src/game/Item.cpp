#include "game/Item.h"

namespace Game3 {
	std::map<ItemID, std::shared_ptr<Item>> Item::ITEMS {
		{Item::SHORTSWORD, std::make_shared<Item>(Item::SHORTSWORD)},
	};

	void to_json(nlohmann::json &json, const ItemStack &stack) {
		json[0] = stack.item->id;
		json[1] = stack.count;
	}

	void from_json(const nlohmann::json &json, ItemStack &stack) {
		stack.item = Item::ITEMS.at(json.at(0));
		stack.count = json.at(1);
	}
}
