#include "ThreadContext.h"
#include "entity/Animal.h"
#include "game/Game.h"
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

	void Animal::wander() {
		timeUntilWander = getWanderDistribution()(threadContext.rng);
		Realm &realm = *getRealm();
		const auto [row, column] = position;
		pathfind({
			std::uniform_int_distribution(std::max(0_idx, row    - wanderRadius), std::min(realm.getHeight() - 1, row    + wanderRadius))(threadContext.rng),
			std::uniform_int_distribution(std::max(0_idx, column - wanderRadius), std::min(realm.getWidth()  - 1, column + wanderRadius))(threadContext.rng)
		}).detach();
	}
}
