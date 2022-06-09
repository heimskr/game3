#include <iostream>

#include "game/Game.h"
#include "tileentity/Building.h"

namespace Game3 {
	constexpr static int WIDTH = 1000;
	
	int getIndex(int r, int c) {
		return r * WIDTH + c;
	}

	void test() {
		// Game game;
		// Texture texture("resources/tileset2.png");
		// auto tilemap = std::make_shared<Tilemap>(WIDTH, WIDTH, 16, texture);
		// auto realm = std::make_shared<Realm>(1, tilemap);
		// game.realms.emplace(realm->id, realm);
		// auto building = std::make_shared<Building>(60, 12, 12, 2);
		// realm->tileEntities.emplace(getIndex(building->row, building->column), building);
		// realm->tilemap1->tiles.at(getIndex(building->row, building->column)) = building->tileID;
		// nlohmann::json json = game;
		// const std::string dumped = json.dump();
		// std::cout << dumped << '\n';
		// std::cout << "JSON size: " << dumped.size() << '\n';
		// std::cout << "CBOR size: " << nlohmann::json::to_cbor(json).size() << '\n';
		// Game thawed = json;
		// std::cout << "Equal? " << (nlohmann::json(thawed).dump() == dumped? "yes" : "no") << '\n';
	}
}
