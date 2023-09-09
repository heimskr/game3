#include "realm/Keep.h"
#include "tileentity/Chest.h"

namespace Game3 {
	Keep::Keep(Game &game_):
		Realm(game_) {}

	Keep::Keep(Game &game_, RealmID id_, Position parent_origin, Index parent_width, Index parent_height, int seed_):
		Realm(game_, id_, ID(), "base:tileset/monomap"_id, seed_),
		parentOrigin(std::move(parent_origin)),
		parentWidth(parent_width),
		parentHeight(parent_height) {}

	void Keep::absorbJSON(const nlohmann::json &json, bool full_data) {
		Realm::absorbJSON(json, full_data);
		parentOrigin = json.at("town").at("origin");
		parentWidth  = json.at("town").at("width");
		parentHeight = json.at("town").at("height");
		money = json.at("money");
		greed = json.at("greed");
		try {
			stockpileInventory = getTileEntity<Chest>([](const auto &chest) { return chest->name == "Stockpile"; })->getInventory();
		} catch (const NoneFoundError &) {
			stockpileInventory = nullptr;
		}
	}

	void Keep::toJSON(nlohmann::json &json, bool full_data) const {
		Realm::toJSON(json, full_data);
		json["town"]["origin"] = parentOrigin;
		json["town"]["width"]  = parentWidth;
		json["town"]["height"] = parentHeight;
		json["money"] = money;
		json["greed"] = greed;
	}
}
