#pragma once

#include <gtkmm.h>
#include <map>
#include <memory>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "Types.h"

namespace Game3 {
	class Texture;

	struct ItemTexture {
		int x;
		int y;
		Texture *texture;
		int width;
		int height;
		ItemTexture() = delete;
		ItemTexture(int x_, int y_, Texture &texture_, int width_ = 16, int height_ = 16): x(x_), y(y_), texture(&texture_), width(width_), height(height_) {}
	};

	enum class ItemAttribute {Axe, Pickaxe, Shovel};

	class Item: public std::enable_shared_from_this<Item> {
		public:
			constexpr static ItemID NOTHING      = 0;
			constexpr static ItemID SHORTSWORD   = 1;
			constexpr static ItemID RED_POTION   = 2;
			constexpr static ItemID COINS        = 3;
			constexpr static ItemID IRON_ORE     = 4;
			constexpr static ItemID COPPER_ORE   = 5;
			constexpr static ItemID GOLD_ORE     = 6;
			constexpr static ItemID DIAMOND_ORE  = 7;
			constexpr static ItemID DIAMOND      = 8;
			constexpr static ItemID COAL         = 9;
			constexpr static ItemID OIL          = 10;
			constexpr static ItemID WOOD         = 11;
			constexpr static ItemID IRON_AXE     = 12;
			constexpr static ItemID IRON_PICKAXE = 13;

			static std::unordered_map<ItemID, ItemTexture> itemTextures;
			static std::map<ItemID, std::shared_ptr<Item>> items;
			static std::unordered_map<ItemID, Durability> durabilities;

			ItemID id = 0;
			std::string name;
			MoneyCount basePrice = 1;
			ItemCount maxCount = 64;
			std::unordered_set<ItemAttribute> attributes;

			Item() = delete;
			Item(ItemID id_, const std::string &name_, MoneyCount base_price, ItemCount max_count = 64): id(id_), name(name_), basePrice(base_price), maxCount(max_count) {}

			Glib::RefPtr<Gdk::Pixbuf> getImage();
			std::shared_ptr<Item> addAttribute(ItemAttribute);
			inline bool operator==(const Item &other) const { return id == other.id; }

		private:
			std::unique_ptr<uint8_t[]> rawImage;
	};

	class ItemStack {
		public:
			std::shared_ptr<Item> item;
			ItemCount count = 1;
			nlohmann::json data;

			ItemStack() = default;
			ItemStack(const std::shared_ptr<Item> &item_, ItemCount count_): item(item_), count(count_) {}
			ItemStack(const std::shared_ptr<Item> &item_, ItemCount count_, const nlohmann::json &data_): item(item_), count(count_), data(data_) {}
			ItemStack(const std::shared_ptr<Item> &item_, ItemCount count_, nlohmann::json &&data_): item(item_), count(count_), data(std::move(data_)) {}
			ItemStack(ItemID id, ItemCount count_): item(Item::items.at(id)), count(count_) {}
			ItemStack(ItemID id, ItemCount count_, const nlohmann::json &data_): item(Item::items.at(id)), count(count_), data(data_) {}
			ItemStack(ItemID id, ItemCount count_, nlohmann::json &&data_): item(Item::items.at(id)), count(count_), data(std::move(data_)) {}

			bool canMerge(const ItemStack &) const;
			Glib::RefPtr<Gdk::Pixbuf> getImage();
			/** Returns a copy of the ItemStack with a different count. */
			ItemStack withCount(ItemCount) const;

			inline bool operator==(const ItemStack &other) const { return *item == *other.item && count == other.count; }

			static ItemStack withDurability(ItemID, Durability durability);
			static ItemStack withDurability(ItemID);

			/** Decreases the durability by a given amount if the ItemStack has durability data. Returns true if the durability was present and reduced to zero or false otherwise. */
			bool reduceDurability(Durability = 1);

		private:
			Glib::RefPtr<Gdk::Pixbuf> cachedImage;
	};

	void to_json(nlohmann::json &, const ItemStack &);
	void from_json(const nlohmann::json &, ItemStack &);
}
