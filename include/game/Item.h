#pragma once

#include <map>
#include <memory>

#include <nlohmann/json.hpp>

#include "Types.h"

namespace Game3 {
	class Item {
		public:
			static std::map<ItemID, std::shared_ptr<Item>> ITEMS;

			constexpr static ItemID NOTHING = 0;
			constexpr static ItemID SHORTSWORD = 1;

			ItemID id = 0;

			Item() = default;
			Item(ItemID id_): id(id_) {}
	};

	class ItemStack {
		public:
			std::shared_ptr<Item> item;
			unsigned count = 1;

			ItemStack() = default;
			ItemStack(const std::shared_ptr<Item> &item_, unsigned count_): item(item_), count(count_) {}
	};

	void to_json(nlohmann::json &, const ItemStack &);
	void from_json(const nlohmann::json &, ItemStack &);
}
