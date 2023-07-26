#pragma once

#include <atomic>
#include <list>
#include <optional>
#include <random>

#include "entity/Entity.h"
#include "threading/ThreadPool.h"

namespace Game3 {
	class Building;

	class Animal: public Entity {
		public:
			static Identifier ID() { return {"base", "entity/animal"}; }
			constexpr static HitPoints MAX_HEALTH = 20;
			constexpr static float RETRY_TIME = 30.f;

			static inline auto getWanderDistribution() {
				return std::uniform_real_distribution(10.f, 20.f);
			}

			Position destination = {-1, -1};
			std::atomic<float> timeUntilWander = 0.f;
			Index wanderRadius = 8;

			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers) override;
			void render(SpriteRenderer &, TextRenderer &) override;
			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void init(Game &) override;
			virtual void tick(Game &, float) override;
			float getSpeed() const override { return 5.f; }
			HitPoints maxHealth() const override { return MAX_HEALTH; }
			bool wander();
			void encode(Buffer &) override;
			void decode(Buffer &) override;

		protected:
			Animal(EntityType);

			std::optional<std::list<Direction>> wanderPath;
			std::atomic_bool attemptingWander = false;

			static ThreadPool threadPool;
	};
}
