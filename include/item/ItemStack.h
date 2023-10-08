#pragma once

#include "Types.h"
#include "container/HeapObject.h"
#include "ui/Modifiers.h"

#include <cassert>
#include <memory>

#include <gtkmm.h>
#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Game;
	class Item;
	class Realm;
	class Texture;
	struct Position;

	class ItemStack {
		public:
			std::shared_ptr<Item> item;
			ItemCount count = 1;
			HeapObject<nlohmann::json> data;

			ItemStack() = default;
			ItemStack(const Game &);
			ItemStack(const Game &, std::shared_ptr<Item> item_, ItemCount count_ = 1);
			ItemStack(const Game &, std::shared_ptr<Item> item_, ItemCount count_, HeapObject<nlohmann::json> data_);
			ItemStack(const Game &, const ItemID &, ItemCount = 1);
			ItemStack(const Game &, const ItemID &, ItemCount, HeapObject<nlohmann::json> data_);

			bool canMerge(const ItemStack &) const;
			Glib::RefPtr<Gdk::Pixbuf> getImage();
			Glib::RefPtr<Gdk::Pixbuf> getImage(const Game &);
			/** Returns a copy of the ItemStack with a different count. */
			ItemStack withCount(ItemCount) const;

			operator std::string() const;

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

			inline const Game & getGame() const { assert(game); return *game; }
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

	std::ostream & operator<<(std::ostream &, const Game3::ItemStack &);
}