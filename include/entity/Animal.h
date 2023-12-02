#pragma once

#include <atomic>
#include <list>
#include <optional>
#include <random>

#include "entity/LivingEntity.h"
#include "threading/ThreadPool.h"

namespace Game3 {
	class Building;

	class Animal: public LivingEntity {
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

			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void init(Game &) override;
			void tick(Game &, float) override;
			float getSpeed() const override { return 5.f; }
			HitPoints getMaxHealth() const override { return MAX_HEALTH; }
			bool wander();
			void encode(Buffer &) override;
			void decode(Buffer &) override;

		protected:
			Animal();

			std::optional<std::list<Direction>> wanderPath;
			std::atomic_bool attemptingWander = false;

			static ThreadPool threadPool;
	};
}
