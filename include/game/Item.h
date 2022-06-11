#pragma once

#include <gtkmm.h>
#include <map>
#include <memory>

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

	class Item {
		public:
			constexpr static ItemID NOTHING    = 0;
			constexpr static ItemID SHORTSWORD = 1;
			constexpr static ItemID RED_POTION = 2;
			constexpr static ItemID COINS      = 3;

			static std::map<ItemID, std::shared_ptr<Item>> items;
			static std::unordered_map<ItemID, ItemTexture> itemTextures;

			ItemID id = 0;
			std::string name;
			MoneyCount basePrice = 1;
			ItemCount maxCount = 64;

			Item() = delete;
			Item(ItemID id_, const std::string &name_, MoneyCount base_price, ItemCount max_count = 64): id(id_), name(name_), basePrice(base_price), maxCount(max_count) {}

			Glib::RefPtr<Gdk::Pixbuf> getImage();

		private:
			std::unique_ptr<uint8_t[]> rawImage;
	};

	class ItemStack {
		public:
			std::shared_ptr<Item> item;
			ItemCount count = 1;

			ItemStack() = default;
			ItemStack(const std::shared_ptr<Item> &item_, ItemCount count_): item(item_), count(count_) {}
			ItemStack(ItemID id, ItemCount count_): item(Item::items.at(id)), count(count_) {}

			bool canMerge(const ItemStack &) const;
			Glib::RefPtr<Gdk::Pixbuf> getImage();
			/** Returns a copy of the ItemStack with a different count. */
			ItemStack withCount(ItemCount) const;

		private:
			Glib::RefPtr<Gdk::Pixbuf> cachedImage;
	};

	void to_json(nlohmann::json &, const ItemStack &);
	void from_json(const nlohmann::json &, ItemStack &);
}
