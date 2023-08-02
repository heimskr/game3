#include "Tileset.h"
#include "game/Game.h"
#include "item/Item.h"
#include "realm/Realm.h"

namespace Game3 {
	void from_json(const nlohmann::json &json, MarchableInfo &marchable_info) {
		if (json.is_string()) {
			marchable_info.corner = json.get<Identifier>();
			marchable_info.categories.clear();
		} else {
			marchable_info.corner     = json.at("corner");
			marchable_info.categories = json.at("categories");
		}
	}

	Tileset::Tileset(Identifier identifier_):
		NamedRegisterable(std::move(identifier_)) {}

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

	const std::unordered_set<Identifier> & Tileset::getBrightNames() const {
		return bright;
	}

	std::vector<TileID> Tileset::getBrightIDs() {
		if (brightCache)
			return *brightCache;

		std::vector<TileID> out;
		out.reserve(bright.size());
		for (const auto &tilename: bright)
			out.push_back(ids.at(tilename));
		brightCache = out;
		return out;
	}

	std::string Tileset::getName() const {
		return name;
	}

	std::shared_ptr<Texture> Tileset::getTexture(const Game &game) {
		if (cachedTexture)
			return cachedTexture;
		return cachedTexture = game.registry<TextureRegistry>()[textureName];
	}

	bool Tileset::getItemStack(const Game &game, const Identifier &id, ItemStack &stack) const {
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

	const MarchableInfo & Tileset::getMarchableInfo(const Identifier &category) const {
		return marchableMap.at(category);
	}

	void Tileset::clearCache() {
		marchableCache.clear();
		unmarchableCache.clear();
		brightCache.reset();
	}

	const std::unordered_set<Identifier> Tileset::getCategories(const Identifier &tilename) const {
		return inverseCategories.at(tilename);
	}

	const std::unordered_set<TileID> Tileset::getCategoryIDs(const Identifier &category) const {
		std::unordered_set<TileID> out;
		for (const auto &tilename: categories.at(category))
			out.insert((*this)[tilename]);
		return out;
	}

	const std::unordered_set<Identifier> Tileset::getTilesByCategory(const Identifier &category) const {
		return categories.at(category);
	}

	bool Tileset::isInCategory(const Identifier &tilename, const Identifier &category) const {
		if (auto iter = inverseCategories.find(tilename); iter != inverseCategories.end())
			return iter->second.contains(category);
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

	const TileID & Tileset::operator[](const Identifier &tilename) const {
		return ids.at(tilename);
	}

	const Identifier & Tileset::operator[](TileID id) const {
		return names.at(id);
	}

	std::optional<TileID> Tileset::maybe(const Identifier &id) const {
		if (auto iter = ids.find(id); iter != ids.end())
			return iter->second;
		return std::nullopt;
	}

	std::optional<std::reference_wrapper<const Identifier>> Tileset::maybe(TileID id) const {
		if (auto iter = names.find(id); iter != names.end())
			return std::ref(iter->second);
		return std::nullopt;
	}

	Tileset Tileset::fromJSON(Identifier identifier, const nlohmann::json &json) {
		Tileset tileset(identifier);
		tileset.tileSize = json.at("tileSize");
		tileset.name = json.at("name");
		tileset.empty = json.at("empty");
		tileset.land = json.at("land");
		tileset.walkable = json.at("walkable");
		tileset.solid = json.at("solid");
		tileset.bright = json.at("bright");
		tileset.ids = json.at("ids");
		tileset.categories = json.at("categories");
		tileset.textureName = json.at("texture");
		for (const auto &[key, value]: json.at("marchable").items()) {
			tileset.marchable.emplace(key);
			if (value.is_string()) {
				auto info = value.get<MarchableInfo>();
				// Make sure the category is in its own set of categories if only the corner is specified.
				info.categories.emplace(key);
				tileset.marchableMap.emplace(Identifier(key), std::move(info));
			} else {
				tileset.marchableMap.emplace(Identifier(key), value.get<MarchableInfo>());
			}
		}

		tileset.missing = json.at("missing");

		std::unordered_map<Identifier, Identifier> stacks = json.at("stacks");
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

		bool changed;
		std::vector<Identifier> add_vector;
		std::vector<Identifier> remove_vector;

		// Please don't introduce any cycles in the category dependency graph.
		do {
			changed = false;

			for (auto &[category, set]: tileset.categories) {
				add_vector.clear();
				remove_vector.clear();

				for (const auto &tilename: set) {
					if (tilename.getPath() == "category") {
						if (auto iter = tileset.categories.find(tilename); iter != tileset.categories.end()) {
							remove_vector.push_back(tilename);
							for (const auto &other: iter->second)
								add_vector.push_back(other);
							changed = true;
						}
					}
				}

				for (const auto &to_add: add_vector)
					set.insert(to_add);

				for (const auto &to_remove: remove_vector)
					set.erase(to_remove);
			}
		} while (changed);

		for (const auto &[category, set]: tileset.categories)
			for (const auto &tilename: set)
				tileset.inverseCategories[tilename].insert(category);

		return tileset;
	}
}
