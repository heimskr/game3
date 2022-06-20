#pragma once

#include <gtkmm.h>
#include <map>
#include <memory>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "Types.h"

namespace Game3 {
	class ItemStack;
	class Player;
	class Realm;
	class Texture;
	struct Position;

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
			constexpr static ItemID NOTHING         = 0;
			constexpr static ItemID SHORTSWORD      = 1;
			constexpr static ItemID RED_POTION      = 2;
			constexpr static ItemID COINS           = 3;
			constexpr static ItemID IRON_ORE        = 4;
			constexpr static ItemID COPPER_ORE      = 5;
			constexpr static ItemID GOLD_ORE        = 6;
			constexpr static ItemID DIAMOND_ORE     = 7;
			constexpr static ItemID DIAMOND         = 8;
			constexpr static ItemID COAL            = 9;
			constexpr static ItemID OIL             = 10;
			constexpr static ItemID WOOD            = 11;
			constexpr static ItemID IRON_AXE        = 12;
			constexpr static ItemID IRON_PICKAXE    = 13;
			constexpr static ItemID IRON_SHOVEL     = 14;
			constexpr static ItemID SAND            = 15;
			constexpr static ItemID STONE           = 16;
			constexpr static ItemID IRON_BAR        = 17;
			constexpr static ItemID SAPLING         = 18;
			constexpr static ItemID GOLD_AXE        = 19;
			constexpr static ItemID GOLD_PICKAXE    = 20;
			constexpr static ItemID GOLD_SHOVEL     = 21;
			constexpr static ItemID GOLD_BAR        = 22;
			constexpr static ItemID DIAMOND_AXE     = 23;
			constexpr static ItemID DIAMOND_PICKAXE = 24;
			constexpr static ItemID DIAMOND_SHOVEL  = 25;
			constexpr static ItemID WOODEN_WALL     = 26;
			constexpr static ItemID PLANK           = 27;

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

			virtual bool use(Slot, ItemStack &, const std::shared_ptr<Player> &, const Position &) { return false; }

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

			inline operator std::string() const { return item->name + " x " + std::to_string(count); }

			inline bool operator==(const ItemStack &other) const { return *item == *other.item && count == other.count; }

			static ItemStack withDurability(ItemID, Durability durability);
			static ItemStack withDurability(ItemID);

			/** Decreases the durability by a given amount if the ItemStack has durability data. Returns true if the durability was present and reduced to zero or false otherwise. */
			bool reduceDurability(Durability = 1);

			bool has(ItemAttribute) const;

			bool hasDurability() const;

			double getDurabilityFraction() const;

			void spawn(const std::shared_ptr<Realm> &, const Position &) const;

		private:
			Glib::RefPtr<Gdk::Pixbuf> cachedImage;
	};

	void to_json(nlohmann::json &, const ItemStack &);
	void from_json(const nlohmann::json &, ItemStack &);
}
