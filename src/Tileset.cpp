#include "Tileset.h"
#include "item/Item.h"
#include "realm/Realm.h"

namespace Game3 {
	bool Tileset::isLand(const Identifier &id) const {
		return land.contains(id);
	}

	bool Tileset::isLand(TileID id) const {
		return isLand(names.at(id));
	}

	bool Tileset::isWalkable(const Identifier &id) const {
		return land.contains(id) || walkable.contains(id) || !solid.contains(id);
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
		return (*this)[empty];
	}

	const Identifier & Tileset::getMissing() const {
		return missing;
	}

	const std::set<Identifier> & Tileset::getBrightNames() const {
		return bright;
	}

	std::vector<TileID> Tileset::getBrightIDs() const {
		std::vector<TileID> out;
		out.reserve(bright.size());
		for (const auto &tilename: bright)
			out.push_back(ids.at(tilename));
		return out;
	}

	std::string Tileset::getName() const {
		return name;
	}

	std::shared_ptr<Texture> Tileset::getTexture() {
		return cacheTexture(std::filesystem::path(texture));
	}

	bool Tileset::getItemStack(Game &game, const Identifier &id, ItemStack &stack) const {
		if (auto iter = inverseCategories.find(id); iter != inverseCategories.end()) {
			for (const auto &category: iter->second) {
				if (auto subiter = stackCategories.find(category); subiter != stackCategories.end()) {
					stack = ItemStack(game, subiter->second);
					return true;
				}
			}
		}

		if (auto iter = stackNames.find(id); iter != stackNames.end()) {
			stack = ItemStack(game, iter->second);
			return true;
		}

		return false;
	}

	bool Tileset::isMarchable(TileID id) {
		if (marchableCache.contains(id))
			return true;

		if (unmarchableCache.contains(id))
			return false;

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

	void Tileset::clearCache() {
		marchableCache.clear();
		unmarchableCache.clear();
	}

	const std::set<Identifier> Tileset::getCategories(const Identifier &tilename) const {
		return inverseCategories.at(tilename);
	}

	const std::set<TileID> Tileset::getCategoryIDs(const Identifier &category) const {
		std::set<TileID> out;
		for (const auto &tilename: categories.at(category))
			out.insert((*this)[tilename]);
		return out;
	}

	const std::set<Identifier> Tileset::getTilesByCategory(const Identifier &category) const {
		return categories.at(category);
	}

	bool Tileset::isInCategory(const Identifier &tilename, const Identifier &category) const {
		return inverseCategories.at(tilename).contains(category);
	}

	const TileID & Tileset::operator[](const Identifier &tilename) const {
		return ids.at(tilename);
	}

	const Identifier & Tileset::operator[](TileID id) const {
		return names.at(id);
	}

	void from_json(const nlohmann::json &json, Tileset &tileset) {
		tileset.name = json.at("name");
		tileset.empty = json.at("empty");
		tileset.land = json.at("land");
		tileset.walkable = json.at("walkable");
		tileset.solid = json.at("solid");
		tileset.bright = json.at("bright");
		tileset.ids = json.at("ids");
		tileset.categories = json.at("categories");
		tileset.texture = json.at("texture");
		tileset.marchable = json.at("marchable");

		std::map<Identifier, Identifier> stacks = json.at("stacks");
		for (const auto &[key, val]: stacks) {
			if (key.getPathStart() == "category")
				tileset.stackCategories.emplace(key, val);
			else
				tileset.stackNames.emplace(key, val);
		}

		for (const auto &[key, val]: tileset.ids) {
			tileset.names.emplace(val, key);
			tileset.inverseCategories.try_emplace(key);
		}

		for (const auto &[category, set]: tileset.categories)
			for (const auto &tilename: set)
				tileset.inverseCategories[tilename].insert(category);
	}
}
