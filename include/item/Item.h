#pragma once

#include <gtkmm.h>
#include <map>
#include <memory>
#include <ostream>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "Types.h"
#include "data/Identifier.h"
#include "registry/Registerable.h"
#include "ui/Modifiers.h"

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
			Item(ItemID, std::string name_, MoneyCount base_price, ItemCount max_count = 64);
			virtual ~Item() = default;

			Item(const Item &) = delete;
			Item(Item &&) = default;

			Item & operator=(const Item &) = delete;
			Item & operator=(Item &&) = default;

			virtual Glib::RefPtr<Gdk::Pixbuf> getImage(const Game &);
			virtual Glib::RefPtr<Gdk::Pixbuf> makeImage(const Game &);
			virtual void getOffsets(const Game &, std::shared_ptr<Texture> &, float &x_offset, float &y_offset);
			std::shared_ptr<Item> addAttribute(Identifier);
			virtual std::shared_ptr<Texture> getTexture(const Game &);
			inline bool operator==(const Item &other) const { return identifier == other.identifier; }

			virtual void initStack(const Game &, ItemStack &) {}

			virtual bool use(Slot, ItemStack &, const Place &, Modifiers, std::pair<float, float> offsets);

			/** Whether the item's use function (see Item::use) should be called when the user interacts with a floor tile and this item is selected in the inventory tab. */
			virtual bool canUseOnWorld() const { return false; }

		protected:
			std::unique_ptr<uint8_t[]> rawImage;
			Glib::RefPtr<Gdk::Pixbuf> cachedImage;
			std::shared_ptr<Texture> cachedTexture;
	};

	class ItemStack {
		public:
			std::shared_ptr<Item> item;
			ItemCount count = 1;
			nlohmann::json data;

			ItemStack() = default;
			ItemStack(const Game &);
			ItemStack(const Game &, std::shared_ptr<Item> item_, ItemCount count_ = 1);
			ItemStack(const Game &, std::shared_ptr<Item> item_, ItemCount count_, nlohmann::json data_);
			ItemStack(const Game &, const ItemID &, ItemCount = 1);
			ItemStack(const Game &, const ItemID &, ItemCount, nlohmann::json data_);

			bool canMerge(const ItemStack &) const;
			Glib::RefPtr<Gdk::Pixbuf> getImage();
			Glib::RefPtr<Gdk::Pixbuf> getImage(const Game &);
			/** Returns a copy of the ItemStack with a different count. */
			ItemStack withCount(ItemCount) const;

			inline operator std::string() const { return item->name + " x " + std::to_string(count); }

			/** Returns true iff the other stack is mergeable with this one and has an equal count. */
			inline bool operator==(const ItemStack &other) const { return canMerge(other) && count == other.count; }

			/** Returns true iff the other stack is mergeable with this one and has a lesser count. */
			inline bool operator<(const ItemStack &other)  const { return canMerge(other) && count <  other.count; }

			/** Returns true iff the other stack is mergeable with this one and has a lesser or equal count. */
			inline bool operator<=(const ItemStack &other) const { return canMerge(other) && count <= other.count; }

			/** Returns true iff the other stack is mergeable with this one and has a greater count. */
			inline bool operator>(const ItemStack &other)  const { return canMerge(other) && count >  other.count; }

			/** Returns true iff the other stack is mergeable with this one and has a greater or equal count. */
			inline bool operator>=(const ItemStack &other) const { return canMerge(other) && count >= other.count; }

			static ItemStack withDurability(const Game &, const ItemID &, Durability durability);
			static ItemStack withDurability(const Game &, const ItemID &);

			/** Decreases the durability by a given amount if the ItemStack has durability data. Returns true if the durability was present and reduced to zero or false otherwise. */
			bool reduceDurability(Durability = 1);
			bool hasAttribute(const Identifier &) const;
			bool hasDurability() const;
			double getDurabilityFraction() const;

			void spawn(const std::shared_ptr<Realm> &, const Position &) const;

			std::shared_ptr<Texture> getTexture(const Game &) const;

			static void fromJSON(const Game &, const nlohmann::json &, ItemStack &);
			static ItemStack fromJSON(const Game &, const nlohmann::json &);
			static std::vector<ItemStack> manyFromJSON(const Game &, const nlohmann::json &);

			void encode(Game &, Buffer &);
			void decode(Game &, Buffer &);

			inline const Game & getGame() { assert(game); return *game; }
			inline bool hasGame() const { return game != nullptr; }

		private:
			const Game *game = nullptr;
			Glib::RefPtr<Gdk::Pixbuf> cachedImage;
			void absorbGame(const Game &);
	};

	template <typename T>
	T popBuffer(Buffer &);
	template <>
	ItemStack popBuffer<ItemStack>(Buffer &);
	Buffer & operator+=(Buffer &, const ItemStack &);
	Buffer & operator<<(Buffer &, const ItemStack &);
	Buffer & operator>>(Buffer &, ItemStack &);

	void to_json(nlohmann::json &, const ItemStack &);
}

std::ostream & operator<<(std::ostream &, const Game3::ItemStack &);
