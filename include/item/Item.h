#pragma once

#include <gtkmm.h>
#include <map>
#include <memory>
#include <ostream>
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

	enum class ItemAttribute {Axe, Pickaxe, Shovel, Hammer, Saw};

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
			constexpr static ItemID CLAY            = 28;
			constexpr static ItemID DIRT            = 29;
			constexpr static ItemID BRICK           = 30;
			constexpr static ItemID POT             = 31;
			constexpr static ItemID PLANT_POT1      = 32;
			constexpr static ItemID PLANT_POT2      = 33;
			constexpr static ItemID PLANT_POT3      = 34;
			constexpr static ItemID IRON_HAMMER     = 35;
			constexpr static ItemID GOLD_HAMMER     = 36;
			constexpr static ItemID DIAMOND_HAMMER  = 37;
			constexpr static ItemID TOWER           = 38;
			constexpr static ItemID CAVE_ENTRANCE   = 39;
			constexpr static ItemID MEAD            = 40;
			constexpr static ItemID HONEY           = 41;
			constexpr static ItemID BOMB            = 42;
			constexpr static ItemID ASH             = 43;
			constexpr static ItemID SAFFRON_MILKCAP = 44;
			constexpr static ItemID HONEY_FUNGUS    = 45;
			constexpr static ItemID BRITTLEGILL     = 46;
			constexpr static ItemID INDIGO_MILKCAP  = 47;
			constexpr static ItemID BLACK_TRUMPET   = 48;
			constexpr static ItemID GREY_KNIGHT     = 49;
			constexpr static ItemID CAULDRON        = 50;

			static std::unordered_map<ItemID, ItemTexture> itemTextures;
			static std::map<ItemID, std::shared_ptr<Item>> items;
			static std::unordered_map<ItemID, Durability> durabilities;

			ItemID id = 0;
			std::string name;
			MoneyCount basePrice = 1;
			ItemCount maxCount = 64;
			std::unordered_set<ItemAttribute> attributes;

			Item() = delete;
			Item(const Item &) = delete;
			Item(Item &&) = default;

			Item & operator=(const Item &) = delete;
			Item & operator=(Item &&) = default;

			Item(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count = 64): id(id_), name(std::move(name_)), basePrice(base_price), maxCount(max_count) {}

			virtual Glib::RefPtr<Gdk::Pixbuf> getImage();
			virtual Glib::RefPtr<Gdk::Pixbuf> makeImage();
			virtual void getOffsets(Texture *&, float &x_offset, float &y_offset);
			std::shared_ptr<Item> addAttribute(ItemAttribute);
			inline bool operator==(const Item &other) const { return id == other.id; }

			virtual bool use(Slot, ItemStack &, const Place &) { return false; }

			/** Whether the item's use function (see Item::use) should be called when the user interacts with a floor tile and this item is selected in the inventory tab. */
			virtual bool canUseOnWorld() const { return false; }

		protected:
			std::unique_ptr<uint8_t[]> rawImage;
			Glib::RefPtr<Gdk::Pixbuf> cachedImage;
	};

	class ItemStack {
		public:
			std::shared_ptr<Item> item;
			ItemCount count = 1;
			nlohmann::json data;

			ItemStack() = default;
			ItemStack(const std::shared_ptr<Item> &item_, ItemCount count_ = 1): item(item_), count(count_) {}
			ItemStack(const std::shared_ptr<Item> &item_, ItemCount count_, const nlohmann::json &data_): item(item_), count(count_), data(data_) {}
			ItemStack(const std::shared_ptr<Item> &item_, ItemCount count_, nlohmann::json &&data_): item(item_), count(count_), data(std::move(data_)) {}
			ItemStack(ItemID id, ItemCount count_ = 1): item(Item::items.at(id)), count(count_) {}
			ItemStack(ItemID id, ItemCount count_, const nlohmann::json &data_): item(Item::items.at(id)), count(count_), data(data_) {}
			ItemStack(ItemID id, ItemCount count_, nlohmann::json &&data_): item(Item::items.at(id)), count(count_), data(std::move(data_)) {}

			bool canMerge(const ItemStack &) const;
			Glib::RefPtr<Gdk::Pixbuf> getImage();
			/** Returns a copy of the ItemStack with a different count. */
			ItemStack withCount(ItemCount) const;

			inline operator std::string() const { return item->name + " x " + std::to_string(count); }

			/** Returns true iff the other stack is mergeable with this one and has an equal count. */
			inline bool operator==(const ItemStack &other) const { return canMerge(other) && count == other.count; }

			/** Returns true iff the other stack is mergeable with this one and has a greater count. */
			inline bool operator<(const ItemStack &other)  const { return canMerge(other) && count <  other.count; }

			/** Returns true iff the other stack is mergeable with this one and has a greater or equal count. */
			inline bool operator<=(const ItemStack &other) const { return canMerge(other) && count <= other.count; }

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

std::ostream & operator<<(std::ostream &, const Game3::ItemStack &);
