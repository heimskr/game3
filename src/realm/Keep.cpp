#include "realm/Keep.h"
#include "tileentity/Chest.h"

namespace Game3 {
	Keep::Keep(RealmID id_, const Position &parent_origin, Index parent_width, Index parent_height, TilemapPtr tilemap1_, TilemapPtr tilemap2_, TilemapPtr tilemap3_, BiomeMapPtr biome_map, int seed_):
		Realm(id_, Realm::KEEP, std::move(tilemap1_), std::move(tilemap2_), std::move(tilemap3_), std::move(biome_map), seed_),
		parentOrigin(parent_origin),
		parentWidth(parent_width),
		parentHeight(parent_height) {}

	Keep::Keep(RealmID id_, const Position &parent_origin, Index parent_width, Index parent_height, TilemapPtr tilemap1_, BiomeMapPtr biome_map, int seed_):
		Realm(id_, Realm::KEEP, std::move(tilemap1_), std::move(biome_map), seed_),
		parentOrigin(parent_origin),
		parentWidth(parent_width),
		parentHeight(parent_height) {}

	void Keep::absorbJSON(const nlohmann::json &json) {
		Realm::absorbJSON(json);
		parentOrigin = json.at("town").at("origin");
		parentWidth  = json.at("town").at("width");
		parentHeight = json.at("town").at("height");
		money = json.at("money");
		greed = json.at("greed");
		stockpileInventory = getTileEntity<Chest>([](const auto &chest) { return chest->name == "Stockpile"; })->inventory;
	}

	void Keep::toJSON(nlohmann::json &json) const {
		Realm::toJSON(json);
		json["town"]["origin"] = parentOrigin;
		json["town"]["width"]  = parentWidth;
		json["town"]["height"] = parentHeight;
		json["money"] = money;
		json["greed"] = greed;
	}
}
