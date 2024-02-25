#pragma once

#include "types/Types.h"
#include "data/Identifier.h"
#include "registry/Registerable.h"
#include "ui/Modifiers.h"

#include <gtkmm.h>
#include <nlohmann/json.hpp>

#include <format>
#include <map>
#include <memory>
#include <ostream>
#include <unordered_set>

namespace Game3 {
	class Game;
	class ItemStack;
	class Player;
	class Realm;
	class Texture;
	struct Position;

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

			virtual Glib::RefPtr<Gdk::Pixbuf> getImage(const Game &, const ItemStack &) const;
			virtual Glib::RefPtr<Gdk::Pixbuf> makeImage(const Game &, const ItemStack &) const;
			virtual Identifier getTextureIdentifier(const ItemStack &) const;
			virtual void getOffsets(const Game &, std::shared_ptr<Texture> &, float &x_offset, float &y_offset);
			Item & addAttribute(Identifier);
			virtual std::shared_ptr<Texture> getTexture(const ItemStack &);
			virtual std::string getTooltip(const ItemStack &);

			inline bool operator==(const Item &other) const { return identifier == other.identifier; }

			virtual void initStack(const Game &, ItemStack &) {}

			/** Called when the user clicks on a tile with the item selected. Returns true iff propagation should stop. */
			virtual bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float> offsets);

			/** Called when the user uses a held item via a keyboard shortcut. Returns true iff propagation should stop. */
			virtual bool use(Slot, ItemStack &, const Place &, Modifiers, Hand hand);

			/** Called when the user uses the item via the context menu or via a keyboard shortcut. Returns true iff propagation should stop. */
			virtual bool use(Slot, ItemStack &, const std::shared_ptr<Player> &, Modifiers);

			virtual bool drag(Slot, ItemStack &, const Place &, Modifiers);

			/** Whether the item's use function (see Item::use) should be called when the user interacts with a floor tile and this item is selected in the inventory tab. */
			virtual bool canUseOnWorld() const { return false; }

			virtual void onDestroy(Game &, ItemStack &) {}

		protected:
			mutable std::unique_ptr<uint8_t[]> rawImage;
			mutable Glib::RefPtr<Gdk::Pixbuf> cachedImage;
			mutable std::shared_ptr<Texture> cachedTexture;
	};

	class ItemStack {
		public:
			std::shared_ptr<Item> item;
			ItemCount count = 1;
			nlohmann::json data;

			ItemStack() = default;
			ItemStack(const std::shared_ptr<Game> &);
			ItemStack(const std::shared_ptr<Game> &, std::shared_ptr<Item> item_, ItemCount count_ = 1);
			ItemStack(const std::shared_ptr<Game> &, std::shared_ptr<Item> item_, ItemCount count_, nlohmann::json data_);
			ItemStack(const std::shared_ptr<Game> &, const ItemID &, ItemCount = 1);
			ItemStack(const std::shared_ptr<Game> &, const ItemID &, ItemCount, nlohmann::json data_);

			bool canMerge(const ItemStack &) const;
			Glib::RefPtr<Gdk::Pixbuf> getImage() const;
			Glib::RefPtr<Gdk::Pixbuf> getImage(const Game &) const;
			/** Returns a copy of the ItemStack with a different count. */
			ItemStack withCount(ItemCount) const;

			inline operator std::string() const { return item->getTooltip(*this) + " x " + std::to_string(count); }

			/** Returns true if the other stack is mergeable with this one and has an equal count. */
			inline bool operator==(const ItemStack &other) const { return canMerge(other) && count == other.count; }

			/** Returns true if the other stack is mergeable with this one and has a lesser count. */
			inline bool operator<(const ItemStack &other)  const { return canMerge(other) && count <  other.count; }

			/** Returns true if the other stack is mergeable with this one and has a lesser or equal count. */
			inline bool operator<=(const ItemStack &other) const { return canMerge(other) && count <= other.count; }

			/** Returns true if the other stack is mergeable with this one and has a greater count. */
			inline bool operator>(const ItemStack &other)  const { return canMerge(other) && count >  other.count; }

			/** Returns true if the other stack is mergeable with this one and has a greater or equal count. */
			inline bool operator>=(const ItemStack &other) const { return canMerge(other) && count >= other.count; }

			static ItemStack withDurability(const std::shared_ptr<Game> &, const ItemID &, Durability durability);
			static ItemStack withDurability(const std::shared_ptr<Game> &, const ItemID &);

			/** Decreases the durability by a given amount if the ItemStack has durability data. Returns true if the durability was present and reduced to zero or false otherwise. */
			bool reduceDurability(Durability = 1);
			bool hasAttribute(const Identifier &) const;
			bool hasDurability() const;
			double getDurabilityFraction() const;
			std::string getTooltip() const;

			inline const auto & getID() const { return item->identifier; }

			void spawn(const std::shared_ptr<Realm> &, const Position &) const;

			std::shared_ptr<Texture> getTexture(Game &) const;

			static void fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &, ItemStack &);
			static ItemStack fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &);
			static std::vector<ItemStack> manyFromJSON(const std::shared_ptr<Game> &, const nlohmann::json &);

			void onDestroy();
			void onDestroy(Game &);

			void encode(Game &, Buffer &);
			void decode(Game &, Buffer &);

			inline std::shared_ptr<Game> getGame() const { auto locked = weakGame.lock(); assert(locked); return locked; }
			inline bool hasGame() const { return !weakGame.expired(); }

		private:
			std::weak_ptr<Game> weakGame;
			mutable Glib::RefPtr<Gdk::Pixbuf> cachedImage;

			void absorbGame(Game &);
	};

	using ItemPtr = std::shared_ptr<Item>;

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	ItemStack popBuffer<ItemStack>(Buffer &);
	Buffer & operator+=(Buffer &, const ItemStack &);
	Buffer & operator<<(Buffer &, const ItemStack &);
	Buffer & operator>>(Buffer &, ItemStack &);
	template <typename T>
	T makeForBuffer(Buffer &);
	template <>
	ItemStack makeForBuffer<ItemStack>(Buffer &);

	void to_json(nlohmann::json &, const ItemStack &);
}

template <>
struct std::formatter<Game3::Item> {
	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
    }

	auto format(const Game3::Item &item, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{}", item.name);
	}
};

template <>
struct std::formatter<Game3::ItemStack> {
	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
    }

	auto format(const Game3::ItemStack &stack, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{} x {}", stack.item->getTooltip(stack), stack.count);
	}
};
