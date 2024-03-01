#pragma once

#include "data/Identifier.h"
#include "registry/Registerable.h"
#include "threading/Lockable.h"
#include "types/Types.h"
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
	struct Place;
	struct Position;
	struct RendererContext;

	class Item: public NamedRegisterable, public std::enable_shared_from_this<Item> {
		public:
			std::string name;
			MoneyCount basePrice = 1;
			ItemCount maxCount = 64;
			std::unordered_set<Identifier> attributes;

			Item() = delete;
			Item(ItemID id_, std::string name_, MoneyCount base_price, ItemCount max_count = 64);
			virtual ~Item();

			Item(const Item &) = delete;
			Item(Item &&) = default;

			Item & operator=(const Item &) = delete;
			Item & operator=(Item &&) = default;

			virtual bool isTextureCacheable() const { return true; }

			virtual Glib::RefPtr<Gdk::Pixbuf> getImage(const Game &, const ConstItemStackPtr &) const;
			virtual Glib::RefPtr<Gdk::Pixbuf> makeImage(const Game &, const ConstItemStackPtr &) const;
			virtual Identifier getTextureIdentifier(const ConstItemStackPtr &) const;
			virtual void getOffsets(const Game &, std::shared_ptr<Texture> &, float &x_offset, float &y_offset);
			Item & addAttribute(Identifier);
			virtual std::shared_ptr<Texture> getTexture(const ConstItemStackPtr &);
			virtual std::string getTooltip(const ConstItemStackPtr &);

			inline bool operator==(const Item &other) const { return identifier == other.identifier; }

			virtual void initStack(const Game &, ItemStack &) {}

			/** Called when the user clicks on a tile with the item selected. Returns true iff propagation should stop. */
			virtual bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float> offsets);

			/** Called when the user uses a held item via a keyboard shortcut. Returns true iff propagation should stop. */
			virtual bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, Hand hand);

			/** Called when the user uses the item via the context menu or via a keyboard shortcut. Returns true iff propagation should stop. */
			virtual bool use(Slot, const ItemStackPtr &, const std::shared_ptr<Player> &, Modifiers);

			virtual bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers);

			/** Whether the item's use function (see Item::use) should be called when the user interacts with a floor tile and this item is selected in the inventory tab. */
			virtual bool canUseOnWorld() const { return false; }

			virtual void onDestroy(Game &, const ItemStackPtr &) const {}

			virtual void renderEffects(const RendererContext &, const Position &, Modifiers, const ItemStackPtr &) const {}

			virtual bool populateMenu(const ItemStackPtr &, Glib::RefPtr<Gio::Menu>, Glib::RefPtr<Gio::SimpleActionGroup>) const { return false; }

		protected:
			mutable std::unique_ptr<uint8_t[]> rawImage;
			mutable Glib::RefPtr<Gdk::Pixbuf> cachedImage;
			mutable std::shared_ptr<Texture> cachedTexture;
	};

	using ItemPtr = std::shared_ptr<Item>;

	class ItemStack: public std::enable_shared_from_this<ItemStack> {
		public:
			std::shared_ptr<Item> item;
			ItemCount count = 1;
			Lockable<nlohmann::json> data;

			template <typename... Args>
			static std::shared_ptr<ItemStack> create(Args &&...args) {
				return std::shared_ptr<ItemStack>(new ItemStack(std::forward<Args>(args)...));
			}

			template <typename... Args>
			static std::shared_ptr<ItemStack> spawn(const Place &place, Args &&...args) {
				auto stack = ItemStack::create(std::forward<Args>(args)...);
				stack->spawn(place);
				return stack;
			}

			bool canMerge(const ItemStack &) const;
			Glib::RefPtr<Gdk::Pixbuf> getImage() const;
			Glib::RefPtr<Gdk::Pixbuf> getImage(const Game &) const;
			/** Returns a copy of the ItemStack with a different count. */
			ItemStackPtr withCount(ItemCount) const;

			inline operator std::string() const { return getTooltip() + " x " + std::to_string(count); }

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

			static ItemStackPtr withDurability(const std::shared_ptr<Game> &, const ItemID &, Durability durability);
			static ItemStackPtr withDurability(const std::shared_ptr<Game> &, const ItemID &);

			/** Decreases the durability by a given amount if the ItemStack has durability data. Returns true if the durability was present and reduced to zero or false otherwise. */
			bool reduceDurability(Durability = 1);
			bool hasAttribute(const Identifier &) const;
			bool hasDurability() const;
			double getDurabilityFraction() const;
			std::string getTooltip() const;

			inline const auto & getID() const { return item->identifier; }

			void spawn(const Place &) const;

			std::shared_ptr<Texture> getTexture(Game &) const;

			static void fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &, ItemStack &);
			static ItemStackPtr fromJSON(const std::shared_ptr<Game> &, const nlohmann::json &);
			static std::vector<ItemStackPtr> manyFromJSON(const std::shared_ptr<Game> &, const nlohmann::json &);

			void onDestroy();
			void onDestroy(Game &);

			void renderEffects(const RendererContext &, const Position &, Modifiers);

			void encode(Game &, Buffer &);
			void decode(Game &, Buffer &);

			std::shared_ptr<ItemStack> copy() const;

			inline std::shared_ptr<Game> getGame() const { auto locked = weakGame.lock(); assert(locked); return locked; }
			inline bool hasGame() const { return !weakGame.expired(); }

		private:
			std::weak_ptr<Game> weakGame;
			mutable Glib::RefPtr<Gdk::Pixbuf> cachedImage;

			ItemStack() = default;
			ItemStack(const std::shared_ptr<Game> &);
			ItemStack(const std::shared_ptr<Game> &, std::shared_ptr<Item> item_, ItemCount count_ = 1);
			ItemStack(const std::shared_ptr<Game> &, std::shared_ptr<Item> item_, ItemCount count_, nlohmann::json data_);
			ItemStack(const std::shared_ptr<Game> &, const ItemID &, ItemCount = 1);
			ItemStack(const std::shared_ptr<Game> &, const ItemID &, ItemCount, nlohmann::json data_);

			void absorbGame(Game &);
	};

	using ItemStackPtr = std::shared_ptr<ItemStack>;
	using ConstItemStackPtr = std::shared_ptr<const ItemStack>;

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	ItemStackPtr popBuffer<ItemStackPtr>(Buffer &);
	Buffer & operator+=(Buffer &, const ItemStackPtr &);
	Buffer & operator<<(Buffer &, const ItemStackPtr &);
	Buffer & operator>>(Buffer &, ItemStackPtr &);
	template <typename T>
	T makeForBuffer(Buffer &);
	template <>
	ItemStackPtr makeForBuffer<ItemStackPtr>(Buffer &);

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
		return std::format_to(ctx.out(), "{} x {}", stack.getTooltip(), stack.count);
	}
};

template <>
struct std::formatter<Game3::ItemStackPtr> {
	constexpr auto parse(std::format_parse_context &ctx) {
		return ctx.begin();
    }

	auto format(const Game3::ItemStackPtr &stack, std::format_context &ctx) const {
		return std::format_to(ctx.out(), "{} x {}", stack->getTooltip(), stack->count);
	}
};
