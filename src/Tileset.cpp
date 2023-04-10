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

	const Identifier & Tileset::getMissing() const {
		return missing;
	}

	std::vector<Identifier> Tileset::getBrightNames() const {
		return {bright.begin(), bright.end()};
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

	Texture & Tileset::getTexture() {
		return cacheTexture(texture);
	}

	bool Tileset::getItemStack(Game &game, const Identifier &id, ItemStack &stack) const {
		if (auto iter = inverseCategories.find(id); iter != inverseCategories.end()) {
			for (const auto &category: iter->second) {
				if (auto subiter = stackCategories.find(id); subiter != stackCategories.end()) {
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

		std::map<Identifier, Identifier> stacks = json.at("stacks");
		for (const auto &[key, val]: stacks) {
			if (key.getPathStart() == "category")
				tileset.stackCategories.emplace(key, val);
			else
				tileset.stackNames.emplace(key, val);
		}

		for (const auto &[key, val]: tileset.ids)
			tileset.names.emplace(val, key);

		for (const auto &[category, set]: tileset.categories)
			for (const auto &tilename: set)
				tileset.inverseCategories[tilename].insert(category);
	}
}
