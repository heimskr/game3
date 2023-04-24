#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class Building;

	class Animal: public Entity {
		public:
			static Identifier ID() { return {"base", "entity/animal"}; }
			constexpr static HitPoints MAX_HEALTH = 20;
			constexpr static float RETRY_TIME = 30.f;

			static inline auto getWanderDistribution() {
				return std::uniform_real_distribution(15.f, 30.f);
			}

			Position destination = {-1, -1};
			float timeUntilWander = 15.f;
			Index wanderRadius = 5;

			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			virtual void tick(Game &, float) override;
			bool wander();

			friend class Entity;

		protected:
			Animal(EntityType);

			HitPoints maxHealth() const override { return MAX_HEALTH; }
			bool stillStuck(float delta);
	};
}
