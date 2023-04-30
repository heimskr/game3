#include "realm/Keep.h"
#include "tileentity/Chest.h"

namespace Game3 {
	Keep::Keep(Game &game_):
		Realm(game_) {}

	Keep::Keep(Game &game_, RealmID id_, const Position &parent_origin, Index parent_width, Index parent_height, int seed_):
		Realm(game_, id_, ID(), seed_),
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
