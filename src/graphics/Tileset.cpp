#include "graphics/Tileset.h"
#include "game/Game.h"
#include "item/Item.h"
#include "realm/Realm.h"
#include "util/Crypto.h"

namespace Game3 {
	Tileset::Tileset(Identifier identifier_):
		NamedRegisterable(std::move(identifier_)) {}

	bool Tileset::isWalkable(const Identifier &id) const {
		return !solid.contains(id);
	}

	bool Tileset::isWalkable(TileID id) const {
		return isWalkable(names.at(id));
	}

	bool Tileset::isSolid(const Identifier &id) const {
		return solid.contains(id);
	}

	bool Tileset::isSolid(TileID id) const {
		return isSolid(names.at(id));
	}

	const Identifier & Tileset::getEmpty() const {
		return empty;
	}

	TileID Tileset::getEmptyID() const {
		if (!emptyID) {
			emptyID = (*this)[empty];
		}

		return *emptyID;
	}

	const Identifier & Tileset::getMissing() const {
		return missing;
	}

	const std::unordered_set<Identifier> & Tileset::getBrightNames() const {
		return bright;
	}

	std::vector<TileID> Tileset::getBrightIDs() {
		if (brightCache) {
			return *brightCache;
		}

		std::vector<TileID> out;
		out.reserve(bright.size());
		for (const auto &tilename: bright) {
			out.push_back(ids.at(tilename));
		}
		brightCache = out;
		return out;
	}

	std::string Tileset::getName() const {
		return name;
	}

	std::shared_ptr<Texture> Tileset::getTexture(const Game &game) {
		if (cachedTexture) {
			return cachedTexture;
		}

		return cachedTexture = game.registry<TextureRegistry>()[textureName];
	}

	bool Tileset::getItemStack(const GamePtr &game, const Identifier &id, ItemStackPtr &stack) const {
		if (auto iter = inverseCategories.find(id); iter != inverseCategories.end()) {
			for (const auto &category: iter->second) {
				if (auto subiter = stackCategories.find(category); subiter != stackCategories.end()) {
					stack = ItemStack::create(game, subiter->second);
					return true;
				}
			}
		}

		if (auto iter = stackNames.find(id); iter != stackNames.end()) {
			stack = ItemStack::create(game, iter->second);
			return true;
		}

		stack = nullptr;
		return false;
	}

	bool Tileset::isMarchable(TileID id) {
		if (marchableCache.contains(id)) {
			return true;
		}

		if (unmarchableCache.contains(id)) {
			return false;
		}

		const auto &tilename = names.at(id);

		if (marchable.contains(tilename)) {
			marchableCache.insert(id);
			return true;
		}

		for (const auto &category: inverseCategories.at(tilename)) {
			if (marchable.contains(category)) {
				marchableCache.insert(id);
				return true;
			}
		}

		unmarchableCache.insert(id);
		return false;
	}

	bool Tileset::isCategoryMarchable(const Identifier &category) const {
		return marchable.contains(category);
	}

	const MarchableInfo * Tileset::getMarchableInfo(const Identifier &tilename) const {
		if (auto iter = marchableMap.find(tilename); iter != marchableMap.end()) {
			return &iter->second;
		}

		return nullptr;
	}

	void Tileset::clearCache() {
		marchableCache.clear();
		unmarchableCache.clear();
		brightCache.reset();
	}

	const std::unordered_set<Identifier> & Tileset::getCategories(const Identifier &tilename) const {
		return inverseCategories.at(tilename);
	}

	const std::unordered_set<TileID> Tileset::getCategoryIDs(const Identifier &category) const {
		std::unordered_set<TileID> out;

		for (const auto &tilename: categories.at(category)) {
			out.insert((*this)[tilename]);
		}

		return out;
	}

	const std::unordered_set<Identifier> & Tileset::getTilesByCategory(const Identifier &category) const {
		return categories.at(category);
	}

	bool Tileset::isInCategory(const Identifier &tilename, const Identifier &category) const {
		if (auto iter = inverseCategories.find(tilename); iter != inverseCategories.end()) {
			return iter->second.contains(category);
		}

		return false;
	}

	bool Tileset::isInCategory(TileID tile_id, const Identifier &category) const {
		return isInCategory((*this)[tile_id], category);
	}

	bool Tileset::hasName(const Identifier &tilename) const {
		return ids.contains(tilename);
	}

	bool Tileset::hasCategory(const Identifier &tilename) const {
		return categories.contains(tilename);
	}

	size_t Tileset::columnCount(const Game &game) {
		return getTexture(game)->width / getTileSize();
	}

	size_t Tileset::rowCount(const Game &game) {
		return getTexture(game)->height / getTileSize();
	}

	void Tileset::setHash(std::string new_hash) {
		hash = std::move(new_hash);
	}

	std::shared_ptr<AutotileSet> Tileset::getAutotileSet(const Identifier &id) const {
		if (id.getPathStart() == "tile") {
			if (auto iter = autotileSetMap.find(id); iter != autotileSetMap.end()) {
				return iter->second;
			}
		} else if (auto iter = autotileSets.find(id); iter != autotileSets.end()) {
			return iter->second;
		}

		return {};
	}

	TileID Tileset::getUpper(TileID id) const {
		if (auto iter = uppers.find(id); iter != uppers.end()) {
			return iter->second;
		}
		return 0;
	}

	bool Tileset::hasUpper(TileID id) const {
		return uppers.contains(id);
	}

	const TileID & Tileset::operator[](const Identifier &tilename) const {
		return ids.at(tilename);
	}

	const Identifier & Tileset::operator[](TileID id) const {
		return names.at(id);
	}

	std::optional<TileID> Tileset::maybe(const Identifier &id) const {
		if (auto iter = ids.find(id); iter != ids.end()) {
			return iter->second;
		}

		return std::nullopt;
	}

	std::optional<std::reference_wrapper<const Identifier>> Tileset::maybe(TileID id) const {
		if (auto iter = names.find(id); iter != names.end()) {
			return std::ref(iter->second);
		}

		return std::nullopt;
	}

	void Tileset::getMeta(boost::json::value &json) const {
		auto &object = json.emplace_object();
		object["hash"] = hexString(hash, false);
		object["names"] = boost::json::value_from(names);
		object["categories"] = boost::json::value_from(categories);
		std::unordered_map<Identifier, Identifier> autotiles;
		for (const auto &[id, autotile]: autotileSetMap) {
			autotiles[id] = autotile->identifier;
		}
		object["autotiles"] = boost::json::value_from(autotiles);
	}


	std::string Tileset::getSQL() {
		return R"(
			CREATE TABLE IF NOT EXISTS tilesets (
				hash VARCHAR(128) PRIMARY KEY,
				json MEDIUMTEXT
			);
		)";
	}

	void Tileset::setAutotile(const Identifier &tilename, const Identifier &autotile_name) {
		if (auto iter = autotileSets.find(autotile_name); iter != autotileSets.end()) {
			autotileSetMap[tilename] = iter->second;
			iter->second->members.insert(tilename);
			return;
		}

		auto autotile_set = std::make_shared<AutotileSet>(AutotileSet{autotile_name, {tilename}});
		autotileSets[autotile_name] = autotile_set;
		autotileSetMap[tilename] = std::move(autotile_set);
	}
}
