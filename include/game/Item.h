#pragma once

#include <map>
#include <memory>

#include <nlohmann/json.hpp>

#include "Types.h"

namespace Game3 {
	class Item {
		public:
			constexpr static ItemID NOTHING = 0;
			constexpr static ItemID SHORTSWORD = 1;
			constexpr static ItemID RED_POTION = 2;

			static std::map<ItemID, std::shared_ptr<Item>> ITEMS;

			ItemID id = 0;
			std::string name;
			unsigned maxCount = 64;

			Item() = default;
			Item(ItemID id_, const std::string &name_, unsigned max_count = 64): id(id_), name(name_), maxCount(max_count) {}
	};

	class ItemStack {
		public:
			std::shared_ptr<Item> item;
			unsigned count = 1;

			ItemStack() = default;
			ItemStack(const std::shared_ptr<Item> &item_, unsigned count_): item(item_), count(count_) {}
			ItemStack(ItemID id, unsigned count_): item(Item::ITEMS.at(id)), count(count_) {}

			bool canMerge(const ItemStack &) const;
	};

	void to_json(nlohmann::json &, const ItemStack &);
	void from_json(const nlohmann::json &, ItemStack &);
}
