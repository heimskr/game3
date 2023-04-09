#pragma once

#include <map>
#include <memory>
#include <set>

#include "Types.h"
#include "registry/Registerable.h"

namespace Game3 {
	class Game;
	class ItemStack;
	class Texture;

	class Tileset: public NamedRegisterable {
		public:
			using NamedRegisterable::NamedRegisterable;

			bool isLand(const Identifier &) const;
			bool isLand(TileID) const;
			bool isWalkable(const Identifier &) const;
			bool isWalkable(TileID) const;
			bool isSolid(const Identifier &) const;
			bool isSolid(TileID) const;
			Identifier getEmpty() const;
			std::vector<Identifier> getBrightNames() const;
			std::vector<TileID> getBrightIDs() const;
			std::string getName() const;
			Texture & getTexture();
			bool getItemStack(Game &, const Identifier &, ItemStack &) const;
			const TileID & operator[](const Identifier &) const;
			const Identifier & operator[](TileID) const;

		private:
			std::string name;
			Identifier empty;
			std::string texture;
			// TODO: consider making the sets store TileIDs instead, for performance perhaps
			std::set<Identifier> land;
			std::set<Identifier> walkable;
			std::set<Identifier> solid;
			std::set<Identifier> bright;
			std::map<Identifier, TileID> ids;
			std::map<TileID, Identifier> names;
			std::map<Identifier, Identifier> stackNames;
			std::map<Identifier, Identifier> stackCategories;
			/** Maps category names to sets of tile names. */
			std::map<Identifier, std::set<Identifier>> categories;
			/** Maps tile names to sets of category names. */
			std::map<Identifier, std::set<Identifier>> inverseCategories;

			friend void from_json(const nlohmann::json &, Tileset &);
	};

	using TilesetPtr = std::shared_ptr<Tileset>;

	void from_json(const nlohmann::json &, Tileset &);
}
