#include "threading/ThreadContext.h"
#include "entity/Animal.h"
#include "game/Game.h"
#include "net/Buffer.h"
#include "realm/Keep.h"
#include "tileentity/Building.h"
#include "tileentity/Chest.h"
#include "tileentity/Teleporter.h"

namespace Game3 {
	ThreadPool Animal::threadPool{2};

	Animal::Animal(EntityType type_):
		Entity(std::move(type_)) {}

	void Animal::toJSON(nlohmann::json &json) const {
		Entity::toJSON(json);

		if (timeUntilWander != 0.f)
			json["timeUntilWander"] = timeUntilWander.load();
	}

	void Animal::absorbJSON(Game &game, const nlohmann::json &json) {
		Entity::absorbJSON(game, json);

		if (auto iter = json.find("timeUntilWander"); iter != json.end())
			timeUntilWander = *iter;
	}

	void Animal::tick(Game &game, float delta) {
		Entity::tick(game, delta);

		if (getSide() == Side::Server) {
			if (!attemptingWander && (timeUntilWander -= delta) <= 0.f)
				wander();
		}
	}

	bool Animal::wander() {
		if (!attemptingWander.exchange(true)) {
			const auto [row, column] = position;
			return threadPool.add([this, row, column](ThreadPool &, size_t) {
				pathfind({
					threadContext.random(static_cast<int64_t>(row    - wanderRadius), static_cast<int64_t>(row    + wanderRadius)),
					threadContext.random(static_cast<int64_t>(column - wanderRadius), static_cast<int64_t>(column + wanderRadius))
				});

				timeUntilWander = getWanderDistribution()(threadContext.rng);
				attemptingWander = false;
			});
		}

		return false;
	}

	void Animal::encode(Buffer &buffer) {
		Entity::encode(buffer);
		buffer << destination;
		buffer << timeUntilWander.load();
		buffer << wanderRadius;
	}

	void Animal::decode(Buffer &buffer) {
		Entity::decode(buffer);
		buffer >> destination;
		timeUntilWander = buffer.take<float>();
		buffer >> wanderRadius;
	}
}
