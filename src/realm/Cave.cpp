#include <iostream>

#include "Tiles.h"
#include "game/Game.h"
#include "realm/Cave.h"
#include "tileentity/Building.h"

namespace Game3 {
	Cave::Cave(RealmID id_, RealmID parent_realm, const std::shared_ptr<Tilemap> &tilemap1_, const std::shared_ptr<Tilemap> &tilemap2_, const std::shared_ptr<Tilemap> &tilemap3_, int seed_):
		Realm(id_, Realm::CAVE, tilemap1_, tilemap2_, tilemap3_, seed_), parentRealm(parent_realm) {}

	Cave::Cave(RealmID id_, RealmID parent_realm, const std::shared_ptr<Tilemap> &tilemap1_, int seed_):
		Realm(id_, Realm::CAVE, tilemap1_, seed_), parentRealm(parent_realm) {}

	Cave::~Cave() {
		// Assumptions:
		// - All entrances to a given cave realm appear in exactly one realm.
		//    -> If we find a cave entrance in one realm, we don't need to search other realms for another entrance to the same cave.
		// - All cave entrances in a given realm lead to the same cave.
		//    -> If we find one cave entrance in a realm, we can stop after destroying its linked cave and we don't have to look for more entrances.
		auto &game = getGame();
		for (const auto &[index, tile_entity]: tileEntities) {
			if (tile_entity->tileID != Monomap::CAVE)
				continue;
			if (auto building = std::dynamic_pointer_cast<Building>(tile_entity)) {
				if (auto cave_realm = std::dynamic_pointer_cast<Cave>(game.realms.at(building->innerRealmID)))
					game.realms.erase(building->innerRealmID);
				else
					std::cerr << "Cave entrance leads to realm " + std::to_string(building->innerRealmID) + ", which isn't a cave. Not erasing.\n";
				break;
			}
		}
	}

	void Cave::absorbJSON(const nlohmann::json &json) {
		Realm::absorbJSON(json);
		parentRealm = json.at("parentRealm");
		entranceCount = json.contains("entranceCount")? json.at("entranceCount").get<decltype(entranceCount)>() : 1;
	}

	void Cave::toJSON(nlohmann::json &json) const {
		Realm::toJSON(json);
		json["parentRealm"] = parentRealm;
		if (entranceCount != 1)
			json["entranceCount"] = entranceCount;
	}
}
