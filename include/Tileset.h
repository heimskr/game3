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
			bool isLand(const Identifier &) const;
			bool isLand(TileID) const;
			bool isWalkable(const Identifier &) const;
			bool isWalkable(TileID) const;
			bool isSolid(const Identifier &) const;
			bool isSolid(TileID) const;
			const Identifier & getEmpty() const;
			TileID getEmptyID() const;
			const Identifier & getMissing() const;
			const std::set<Identifier> & getBrightNames() const;
			std::vector<TileID> getBrightIDs();
			std::string getName() const;
			std::shared_ptr<Texture> getTexture(const Game &);
			const Identifier & getTextureName() const { return textureName; }
			bool getItemStack(Game &, const Identifier &, ItemStack &) const;
			bool isMarchable(TileID);
			bool isCategoryMarchable(const Identifier &category) const;
			void clearCache();
			const std::set<Identifier> getCategories(const Identifier &) const;
			const std::set<TileID> getCategoryIDs(const Identifier &) const;
			const std::set<Identifier> getTilesByCategory(const Identifier &) const;
			bool isInCategory(const Identifier &tilename, const Identifier &category) const;
			bool hasName(const Identifier &) const;
			const TileID & operator[](const Identifier &) const;
			const Identifier & operator[](TileID) const;

			static Tileset fromJSON(Identifier, const nlohmann::json &);

		private:
			Tileset(Identifier identifier_);
			std::string name;
			Identifier empty;
			Identifier missing;
			Identifier textureName;
			// TODO: consider making the sets store TileIDs instead, for performance perhaps
			std::set<Identifier> land;
			std::set<Identifier> walkable;
			std::set<Identifier> solid;
			std::set<Identifier> bright;
			std::set<Identifier> marchable;
			std::map<Identifier, TileID> ids;
			std::map<TileID, Identifier> names;
			std::map<Identifier, Identifier> stackNames;
			std::map<Identifier, Identifier> stackCategories;
			/** Maps category names to sets of tile names. */
			std::map<Identifier, std::set<Identifier>> categories;
			/** Maps tile names to sets of category names. */
			std::map<Identifier, std::set<Identifier>> inverseCategories;
			std::set<TileID> marchableCache;
			std::set<TileID> unmarchableCache;
			std::optional<std::vector<TileID>> brightCache;
	};

	using TilesetPtr = std::shared_ptr<Tileset>;
}
