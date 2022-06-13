#include <iostream>

#include "Tiles.h"
#include "entity/Gatherer.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "tileentity/Teleporter.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "util/Util.h"

namespace Game3 {
	Gatherer::Gatherer(EntityID id_):
		Entity(id_) {}

	Gatherer::Gatherer(EntityID id_, RealmID overworld_realm, RealmID house_realm, const Position &house_position):
		Entity(id_), overworldRealm(overworld_realm), houseRealm(house_realm), housePosition(house_position) {}

	std::shared_ptr<Gatherer> Gatherer::create(EntityID id, RealmID overworld_realm, RealmID house_realm, const Position &house_position) {
		auto out = std::shared_ptr<Gatherer>(new Gatherer(id, overworld_realm, house_realm, house_position));
		out->init();
		return out;
	}

	std::shared_ptr<Gatherer> Gatherer::fromJSON(const nlohmann::json &json) {
		auto out = Entity::create<Gatherer>(json.at("id"));
		out->absorbJSON(json);
		return out;
	}

	nlohmann::json Gatherer::toJSON() const {
		nlohmann::json json;
		to_json(json, *this);
		return json;
	}

	void Gatherer::absorbJSON(const nlohmann::json &json) {
		Entity::absorbJSON(json);
		phase          = json.at("phase");
		overworldRealm = json.at("overworldRealm");
		houseRealm     = json.at("house").at(0);
		housePosition  = json.at("house").at(1);
		harvestingTime = json.at("harvestingTime");
		if (json.contains("chosenResouce"))
			chosenResource = json.at("chosenResource");
	}

	void Gatherer::tick(Game &game, float delta) {
		Entity::tick(game, delta);
		const auto hour = game.getHour();
		if (8. <= hour && phase == 0) {
			phase = 1;
			auto &overworld = *game.realms.at(overworldRealm);
			auto &house     = *game.realms.at(houseRealm);
			const auto width  = overworld.getWidth();
			const auto height = overworld.getHeight();
			const auto &layer2 = *overworld.tilemap2;
			// Detect all resources within a given radius of the house
			std::vector<Index> resource_choices;
			for (Index row = 0; row < height; ++row)
				for (Index column = 0; column < width; ++column)
					if (overworldTiles.isResource(layer2(column, row)) && std::sqrt(std::pow(housePosition.row - row, 2) + std::pow(housePosition.column - column, 2)) <= RADIUS)
						resource_choices.push_back(overworld.getIndex(row, column));
			// Choose one at random
			chosenResource = choose(resource_choices, game.dynamicRNG);
			// Pathfind to the door
			pathfind(house.getTileEntity<Teleporter>()->position);
		} else if (16. <= hour && phase == 3) {
			phase = 4;
		}

		if (phase == 1 && realmID == overworldRealm) {
			auto &realm = *getRealm();
			auto chosen_position = realm.getPosition(chosenResource);
			if (auto next = realm.getPathableAdjacent(chosen_position)) {
				phase = 2;
				pathfind(chosen_position);
			} else
				phase = -1;
		}

		if (phase == 2 && path.empty()) {
			phase = 3;
			harvestingTime = 0.f;
		}

		if (phase == 3) {
			if (HARVESTING_TIME <= harvestingTime) {
				auto &realm = *getRealm();
				const auto resource_position = realm.getPosition(chosenResource);
				const TileID resource_type = (*getRealm()->tilemap2)(resource_position.column, resource_position.row);
				switch (resource_type) {
					default:
						throw std::runtime_error("Unknown resource type: " + std::to_string(resource_type));
				}

				harvestingTime = 0.f;
			} else
				harvestingTime += delta;
		}
	}

	void to_json(nlohmann::json &json, const Gatherer &gatherer) {
		to_json(json, static_cast<const Entity &>(gatherer));
		json["phase"] = gatherer.phase;
		json["overworldRealm"] = gatherer.overworldRealm;
		json["house"][0] = gatherer.houseRealm;
		json["house"][1] = gatherer.housePosition;
		json["harvestingTime"] = gatherer.harvestingTime;
		if (gatherer.chosenResource != -1)
			json["chosenResource"] = gatherer.chosenResource;
	}
}
