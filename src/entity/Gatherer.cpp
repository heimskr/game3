#include <iostream>

#include "Tiles.h"
#include "entity/Gatherer.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "ui/Canvas.h"
#include "ui/MainWindow.h"
#include "util/Util.h"

namespace Game3 {
	std::shared_ptr<Gatherer> Gatherer::create(EntityID id) {
		auto out = std::shared_ptr<Gatherer>(new Gatherer(id));
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
		phase = json.at("phase");
		overworldRealm = json.at("overworldRealm");
		housePosition  = json.at("housePosition");
	}

	void Gatherer::tick(Game &game, float delta) {
		Entity::tick(game, delta);
		const auto hour = game.getHour();
		if (8. <= hour && phase == 0) {
			phase = 1;
			auto &overworld = *game.realms.at(overworldRealm);
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
			const Index choice = choose(resource_choices, game.dynamicRNG);
		} else if (16. <= hour && phase == 3) {
			phase = 4;
		}
	}

	void to_json(nlohmann::json &json, const Gatherer &gatherer) {
		to_json(json, static_cast<const Entity &>(gatherer));
		json["phase"] = gatherer.phase;
		json["overworldRealm"] = gatherer.overworldRealm;
		json["housePosition"]  = gatherer.housePosition;
	}
}
