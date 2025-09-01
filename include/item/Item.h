#pragma once

#include "data/Identifier.h"
#include "registry/Registerable.h"
#include "threading/Lockable.h"
#include "types/Types.h"
#include "ui/Modifiers.h"

#include <boost/json.hpp>

#include <format>
#include <map>
#include <memory>
#include <ostream>
#include <unordered_set>

namespace Game3 {
	class Game;
	class ItemStack;
	class ItemTexture;
	class Player;
	class Realm;
	class Texture;
	class Window;
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

			virtual TexturePtr getTexture(const Game &, const ConstItemStackPtr &) const;
			virtual TexturePtr makeTexture(const Game &, const ConstItemStackPtr &) const;
			virtual Identifier getTextureIdentifier(const ConstItemStackPtr &) const;
			virtual void getOffsets(const Game &, std::shared_ptr<Texture> &, float &x_offset, float &y_offset);
			virtual Item & addAttribute(Identifier) &;
			virtual Item && addAttribute(Identifier) &&;
			virtual bool hasAttribute(const Identifier &) const;
			virtual std::shared_ptr<ItemTexture> getItemTexture(const ConstItemStackPtr &);
			virtual std::string getTooltip(const ConstItemStackPtr &);

			inline bool operator==(const Item &other) const { return identifier == other.identifier; }

			virtual void initStack(const Game &, ItemStack &) {}

			/** Called when the user clicks on a tile with the item selected. Returns true iff propagation should stop. */
			virtual bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float> offsets);

			/** Called when the user uses a held item via a keyboard shortcut. Returns true iff propagation should stop. */
			virtual bool use(Slot, const ItemStackPtr &, const Place &, Modifiers, Hand hand);

			/** Called when the user uses the item via the context menu or via a keyboard shortcut. Returns true iff propagation should stop. */
			virtual bool use(Slot, const ItemStackPtr &, const std::shared_ptr<Player> &, Modifiers);

			virtual bool drag(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float> offsets, DragAction);

			/** Called on the client every tick while the user is holding the mouse button on the world with the item selected. */
			virtual bool fire(Slot, const ItemStackPtr &, const Place &, Modifiers, std::pair<float, float> offsets);

			/** Whether the item's use function (see Item::use) should be called when the user interacts with a floor tile and this item is selected in the inventory tab. */
			virtual bool canUseOnWorld() const { return false; }

			virtual void onDestroy(Game &, const ItemStackPtr &) const {}

			virtual void renderEffects(Window &, const RendererContext &, const Position &, Modifiers, const ItemStackPtr &) const {}

			// virtual bool populateMenu(const InventoryPtr &, Slot, const ItemStackPtr &, Glib::RefPtr<Gio::Menu>, Glib::RefPtr<Gio::SimpleActionGroup>) const { return false; }

		protected:
			mutable Lockable<std::shared_ptr<uint8_t[]>> rawImage;
			mutable std::shared_ptr<ItemTexture> cachedItemTexture;
			mutable TexturePtr cachedTexture;
	};

	using ItemPtr = std::shared_ptr<Item>;

	class ItemStack: public std::enable_shared_from_this<ItemStack> {
		public:
			std::shared_ptr<Item> item;
			ItemCount count = 1;
			Lockable<boost::json::value> data;

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

			TexturePtr getTexture() const;
			TexturePtr getTexture(const Game &) const;

			bool canMerge(const ItemStack &) const;
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

			std::shared_ptr<ItemTexture> getItemTexture(Game &) const;

			static std::vector<ItemStackPtr> manyFromJSON(const std::shared_ptr<Game> &, const boost::json::value &);

			void onDestroy();
			void onDestroy(Game &);

			void renderEffects(Window &, const RendererContext &, const Position &, Modifiers);

			void encode(Game &, Buffer &);
			void decode(Game &, BasicBuffer &);

			std::shared_ptr<ItemStack> copy() const;

			inline std::shared_ptr<Game> getGame() const { auto locked = weakGame.lock(); assert(locked); return locked; }
			inline bool hasGame() const { return !weakGame.expired(); }

		private:
			std::weak_ptr<Game> weakGame;
			mutable TexturePtr cachedTexture;

			ItemStack() = default;
			ItemStack(const std::shared_ptr<Game> &);
			ItemStack(const std::shared_ptr<Game> &, std::shared_ptr<Item> item_, ItemCount count_ = 1);
			ItemStack(const std::shared_ptr<Game> &, std::shared_ptr<Item> item_, ItemCount count_, boost::json::value data_);
			ItemStack(const std::shared_ptr<Game> &, const ItemID &, ItemCount = 1);
			ItemStack(const std::shared_ptr<Game> &, const ItemID &, ItemCount, boost::json::value data_);

			void absorbGame(Game &);

		friend class Buffer;
	};

	using ItemStackPtr = std::shared_ptr<ItemStack>;
	using ConstItemStackPtr = std::shared_ptr<const ItemStack>;

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	ItemStackPtr popBuffer<ItemStackPtr>(Buffer &);
	Buffer & operator+=(Buffer &, const ItemStackPtr &);
	Buffer & operator<<(Buffer &, const ItemStackPtr &);
	BasicBuffer & operator>>(BasicBuffer &, ItemStackPtr &);
	template <typename T>
	T makeForBuffer(Buffer &);
	template <>
	ItemStackPtr makeForBuffer<ItemStackPtr>(Buffer &);

	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const ItemStack &);
	void tag_invoke(boost::json::value_from_tag, boost::json::value &, const ItemStackPtr &);
	ItemStackPtr tag_invoke(boost::json::value_to_tag<ItemStackPtr>, const boost::json::value &, const GamePtr &);
}

template <>
struct std::formatter<Game3::Item> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const Game3::Item &item, auto &ctx) const {
		return std::format_to(ctx.out(), "{}", item.name);
	}
};

template <>
struct std::formatter<Game3::ItemStack> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const Game3::ItemStack &stack, auto &ctx) const {
		return std::format_to(ctx.out(), "{} x {}", stack.getTooltip(), stack.count);
	}
};

template <>
struct std::formatter<Game3::ItemStackPtr> {
	constexpr auto parse(auto &ctx) {
		return ctx.begin();
	}

	auto format(const Game3::ItemStackPtr &stack, auto &ctx) const {
		return std::format_to(ctx.out(), "{} x {}", stack->getTooltip(), stack->count);
	}
};
