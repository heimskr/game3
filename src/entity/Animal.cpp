#include "ThreadContext.h"
#include "entity/Animal.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "realm/Keep.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/Teleporter.h"

namespace Game3 {
	Animal::Animal(EntityType type_):
		Entity(std::move(type_)) {}

	void Animal::toJSON(nlohmann::json &json) const {
		Entity::toJSON(json);

		if (timeUntilWander != 0.f)
			json["timeUntilWander"] = timeUntilWander;
	}

	void Animal::absorbJSON(Game &game, const nlohmann::json &json) {
		Entity::absorbJSON(game, json);

		if (auto iter = json.find("timeUntilWander"); iter != json.end())
			timeUntilWander = *iter;
	}

	void Animal::tick(Game &game, float delta) {
		Entity::tick(game, delta);

		if ((timeUntilWander -= delta) <= 0.f)
			wander();
	}

	bool Animal::wander() {
		timeUntilWander = getWanderDistribution()(threadContext.rng);
		const auto [row, column] = position;
		return pathfind({
			std::uniform_int_distribution(row    - wanderRadius, row    + wanderRadius)(threadContext.rng),
			std::uniform_int_distribution(column - wanderRadius, column + wanderRadius)(threadContext.rng)
		});
	}

	void Animal::encode(Buffer &buffer) {
		Entity::encode(buffer);
		buffer << destination;
		buffer << timeUntilWander;
		buffer << wanderRadius;
	}

	void Animal::decode(Buffer &buffer) {
		Entity::decode(buffer);
		buffer >> destination;
		buffer >> timeUntilWander;
		buffer >> wanderRadius;
	}
}
