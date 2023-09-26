#pragma once

#include "Types.h"
#include "registry/Registerable.h"

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json_fwd.hpp>

namespace Game3 {
	class Game;
	class ItemStack;
	class Texture;

	struct MarchableInfo {
		/** The top-left corner of the set of marchable tiles. */
		Identifier corner;
		/** A set of categories of tiles that this marchable category should consider possible neighbors. */
		std::unordered_set<Identifier> categories;

		MarchableInfo() = default;
		MarchableInfo(Identifier corner_, std::unordered_set<Identifier> categories_):
			corner(std::move(corner_)), categories(std::move(categories_)) {}
	};

	void from_json(const nlohmann::json &, MarchableInfo &);

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
			const std::unordered_set<Identifier> & getBrightNames() const;
			std::vector<TileID> getBrightIDs();
			std::string getName() const;
			std::shared_ptr<Texture> getTexture(const Game &);
			const Identifier & getTextureName() const { return textureName; }
			bool getItemStack(const Game &, const Identifier &, ItemStack &) const;
			bool isMarchable(TileID);
			bool isCategoryMarchable(const Identifier &category) const;
			const Identifier & getMarchCorner(const Identifier &category) const;
			const MarchableInfo & getMarchableInfo(const Identifier &category) const;
			void clearCache();
			const std::unordered_set<Identifier> getCategories(const Identifier &) const;
			const std::unordered_set<TileID> getCategoryIDs(const Identifier &) const;
			const std::unordered_set<Identifier> getTilesByCategory(const Identifier &) const;
			bool isInCategory(const Identifier &tilename, const Identifier &category) const;
			bool isInCategory(TileID, const Identifier &category) const;
			bool hasName(const Identifier &) const;
			bool hasCategory(const Identifier &) const;
			inline auto getTileSize() const { return tileSize; }
			size_t columnCount(const Game &);
			size_t rowCount(const Game &);

			const auto & getIDs() { return ids; }
			const auto & getNames() { return names; }
			const auto & getLand() { return land; }
			const auto & getWalkable() { return walkable; }
			const auto & getSolid() { return solid; }
			const auto & getMarchable() { return marchableMap; }

			const TileID & operator[](const Identifier &) const;
			const Identifier & operator[](TileID) const;

			std::optional<TileID> maybe(const Identifier &) const;
			std::optional<std::reference_wrapper<const Identifier>> maybe(TileID) const;

			static Tileset fromJSON(Identifier, const nlohmann::json &);

		private:
			Tileset(Identifier identifier_);
			std::string name;
			size_t tileSize = 0;
			Identifier empty;
			Identifier missing;
			Identifier textureName;
			std::shared_ptr<Texture> cachedTexture;
			// TODO: consider making the sets store TileIDs instead, for performance perhaps
			std::unordered_set<Identifier> land;
			std::unordered_set<Identifier> walkable;
			std::unordered_set<Identifier> solid;
			std::unordered_set<Identifier> bright;
			std::unordered_set<Identifier> marchable;
			std::unordered_map<Identifier, MarchableInfo> marchableMap;
			std::unordered_map<Identifier, TileID> ids;
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

		friend void splitter();
		friend Tileset stitcher(const std::filesystem::path &, Identifier, std::string *);
	};

	using TilesetPtr = std::shared_ptr<Tileset>;
}
