#pragma once

#include "entity/Monster.h"

namespace Game3 {
	class Building;

	class Cyclops: public Monster {
		public:
			static Identifier ID() { return {"base", "entity/cyclops"}; }

			static std::shared_ptr<Cyclops> create(const std::shared_ptr<Game> &) {
				return Entity::create<Cyclops>();
			}

			static std::shared_ptr<Cyclops> fromJSON(const std::shared_ptr<Game> &game, const boost::json::value &json) {
				auto out = Entity::create<Cyclops>();
				out->absorbJSON(game, json);
				return out;
			}

			std::string getName() const override { return "Cyclops"; }
			float getMovementSpeed() const override { return 8.f; }

			HitPoints getBaseDamage() const override { return 2; }
			int getVariability() const override { return 1; }
			float getAttackPeriod() const override { return 1.f; }

		protected:
			Cyclops():
				Entity(ID()), Monster() { luckStat = 0.5; }

		friend class Entity;
	};
}
