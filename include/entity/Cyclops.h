#pragma once

#include "entity/Monster.h"

namespace Game3 {
	class Building;

	class Cyclops: public Monster {
		public:
			static Identifier ID() { return {"base", "entity/cyclops"}; }

			static std::shared_ptr<Cyclops> create(Game &) {
				return Entity::create<Cyclops>();
			}

			static std::shared_ptr<Cyclops> fromJSON(Game &game, const nlohmann::json &json) {
				auto out = Entity::create<Cyclops>();
				out->absorbJSON(game, json);
				return out;
			}

			std::string getName() const override { return "Cyclops"; }
			float getSpeed() const override { return 8.f; }

			HitPoints getBaseDamage() const override { return 2; }
			int getVariability() const override { return 1; }
			float getAttackPeriod() const override { return 2.f; }

		protected:
			Cyclops():
				Entity(ID()), Monster() { luckStat = 0.5; }

		friend class Entity;
	};
}
