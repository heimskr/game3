#pragma once

#include "Types.h"
#include "data/Identifier.h"
#include "registry/Registerable.h"
#include "ui/Modifiers.h"

#include <gtkmm.h>
#include <map>
#include <memory>
#include <ostream>
#include <unordered_set>

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Game;
	class ItemStack;
	class Player;
	class Realm;
	class Texture;
	struct Position;

	struct ItemTexture: NamedRegisterable {
		static constexpr int DEFAULT_WIDTH = 16;
		static constexpr int DEFAULT_HEIGHT = 16;

		int x = -1;
		int y = -1;
		Identifier textureName;
		std::weak_ptr<Texture> texture;
		int width = -1;
		int height = -1;

		ItemTexture() = delete;
		ItemTexture(Identifier, Identifier texture_name, int x_, int y_, int width_ = DEFAULT_WIDTH, int height_ = DEFAULT_HEIGHT);

		std::shared_ptr<Texture> getTexture(const Game &);

		explicit operator bool() const;
	};

	void to_json(nlohmann::json &, const ItemTexture &);

	class Item: public NamedRegisterable, public std::enable_shared_from_this<Item> {
		public:
			std::string name;
			MoneyCount basePrice = 1;
			ItemCount maxCount = 64;
			std::unordered_set<Identifier> attributes;

			Item() = delete;
			Item(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count = 64);
			virtual ~Item() = default;

			Item(const Item &) = delete;
			Item(Item &&) = default;

			Item & operator=(const Item &) = delete;
			Item & operator=(Item &&) = default;

			virtual bool isTextureCacheable() const { return true; }

			virtual Glib::RefPtr<Gdk::Pixbuf> getImage(const Game &, const ItemStack &);
			virtual Glib::RefPtr<Gdk::Pixbuf> makeImage(const Game &, const ItemStack &);
			virtual Identifier getTextureIdentifier(const ItemStack &);
			virtual void getOffsets(const Game &, std::shared_ptr<Texture> &, float &x_offset, float &y_offset);
			std::shared_ptr<Item> addAttribute(Identifier);
			virtual std::shared_ptr<Texture> getTexture(const ItemStack &);
			virtual std::string getTooltip(const ItemStack &);

			inline bool operator==(const Item &other) const { return identifier == other.identifier; }

			virtual void initStack(const Game &, ItemStack &) {}

			virtual bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float> offsets);

			virtual bool drag(Slot, ItemStack &, const Place &, Modifiers);

			/** Whether the item's use function (see Item::use) should be called when the user interacts with a floor tile and this item is selected in the inventory tab. */
			virtual bool canUseOnWorld() const { return false; }

		protected:
			std::unique_ptr<uint8_t[]> rawImage;
			Glib::RefPtr<Gdk::Pixbuf> cachedImage;
			std::shared_ptr<Texture> cachedTexture;
	};
}
