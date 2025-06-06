#pragma once

#include "types/Types.h"
#include "registry/Registerable.h"

#include <boost/json/fwd.hpp>

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Game3 {
	class Texture;

	struct AutotileSet {
		Identifier identifier;
		std::unordered_set<Identifier> members;
		bool omni = false;
	};

	struct MarchableInfo {
		/** The left edge of the row of marchable tiles. */
		Identifier start;
		/** The set of possible neighbors. */
		std::shared_ptr<AutotileSet> autotileSet;
		bool tall = false;
		/** Whether it's for 8-way autotiling instead of 4-way. */
		bool eight = false;

		MarchableInfo() = default;
		MarchableInfo(Identifier start, std::shared_ptr<AutotileSet> autotileSet, bool tall, bool eight):
			start(std::move(start)),
			autotileSet(std::move(autotileSet)),
			tall(tall),
			eight(eight) {}
	};

	class Tileset: public NamedRegisterable {
		public:
			bool isWalkable(const Identifier &) const;
			bool isWalkable(TileID) const;
			bool isSolid(const Identifier &) const;
			bool isSolid(TileID) const;
			const Identifier & getEmpty() const;
			TileID getEmptyID() const;
			const Identifier & getMissing() const;
			const std::unordered_set<Identifier> & getBrightNames() const;
			std::vector<TileID> getBrightIDs();
			std::string getName() const;
			std::shared_ptr<Texture> getTexture(const Game &);
			const Identifier & getTextureName() const { return textureName; }
			bool getItemStack(const std::shared_ptr<Game> &, const Identifier &, ItemStackPtr &) const;
			bool isMarchable(TileID);
			bool isCategoryMarchable(const Identifier &category) const;
			const MarchableInfo * getMarchableInfo(const Identifier &tilename) const;
			void clearCache();
			const std::unordered_set<Identifier> & getCategories(const Identifier &) const;
			const std::unordered_set<TileID> getCategoryIDs(const Identifier &) const;
			const std::unordered_set<Identifier> & getTilesByCategory(const Identifier &) const;
			bool isInCategory(const Identifier &tilename, const Identifier &category) const;
			bool isInCategory(TileID, const Identifier &category) const;
			bool hasName(const Identifier &) const;
			bool hasCategory(const Identifier &) const;
			inline auto getTileSize() const { return tileSize; }
			size_t columnCount(const Game &);
			size_t rowCount(const Game &);
			void setHash(std::string);
			std::shared_ptr<AutotileSet> getAutotileSet(const Identifier &) const;
			TileID getUpper(TileID) const;
			bool hasUpper(TileID) const;

			const auto & getIDs()          const { return ids;          }
			const auto & getNames()        const { return names;        }
			const auto & getSolid()        const { return solid;        }
			const auto & getMarchable()    const { return marchableMap; }
			const auto & getHash()         const { return hash;         }
			const auto & getAutotileSets() const { return autotileSets; }

			const TileID & operator[](const Identifier &) const;
			const Identifier & operator[](TileID) const;

			std::optional<TileID> maybe(const Identifier &) const;
			std::optional<std::reference_wrapper<const Identifier>> maybe(TileID) const;

			/** Produces a limited amount of JSON about the tileset. */
			void getMeta(boost::json::value &) const;

			static std::string getSQL();

		private:
			Tileset(Identifier identifier_);

			std::string name;
			std::string hash;
			size_t tileSize = 0;
			Identifier empty;
			Identifier missing;
			Identifier textureName;
			mutable std::optional<TileID> emptyID;

			std::shared_ptr<Texture> cachedTexture;
			// TODO: consider making the sets store TileIDs instead, for performance perhaps
			std::unordered_set<Identifier> solid;
			std::unordered_set<Identifier> bright;
			std::unordered_set<Identifier> marchable;
			std::unordered_map<Identifier, MarchableInfo> marchableMap;
			/** Maps tilename identifiers to numeric IDs. */
			std::unordered_map<Identifier, TileID> ids;
			/** Maps numeric IDs to tilename identifiers. */
			std::unordered_map<TileID, Identifier> names;
			std::unordered_map<Identifier, Identifier> stackNames;
			std::unordered_map<Identifier, Identifier> stackCategories;
			/** Maps category names to sets of tile names. */
			std::unordered_map<Identifier, std::unordered_set<Identifier>> categories;
			/** Maps tile names to sets of category names. */
			std::unordered_map<Identifier, std::unordered_set<Identifier>> inverseCategories;
			std::unordered_set<TileID> marchableCache;
			std::unordered_set<TileID> unmarchableCache;
			std::optional<std::vector<TileID>> brightCache;
			/** Maps autotile identifiers to autotile set pointers. */
			std::unordered_map<Identifier, std::shared_ptr<AutotileSet>> autotileSets;
			/** Maps tilenames to autotile set pointers. */
			std::unordered_map<Identifier, std::shared_ptr<AutotileSet>> autotileSetMap;
			/** Maps base tile IDs for the lower portions of tall tiles to the tile IDs for the upper portions. */
			std::unordered_map<TileID, TileID> uppers;

			void setAutotile(const Identifier &tilename, const Identifier &autotile_name);

		friend Tileset tileStitcher(const std::filesystem::path &, Identifier, Side, std::string *);
	};

	using TilesetPtr = std::shared_ptr<Tileset>;
}
