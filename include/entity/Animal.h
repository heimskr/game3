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
			static inline auto getWanderDistribution() {
				return std::uniform_real_distribution(10.f, 20.f);
			}

			Index wanderRadius = 8;

			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, ItemStack *, Hand) override;
			void init(Game &) override;
			void tick(const TickArgs &) override;
			float getMovementSpeed() const override { return 5.f; }
			HitPoints getMaxHealth() const override;
			bool wander();
			void encode(Buffer &) override;
			void decode(Buffer &) override;

		protected:
			Animal();

			bool firstWander = true;
			Tick wanderTick = 0;
			std::optional<std::list<Direction>> wanderPath;
			std::atomic_bool attemptingWander = false;

			static ThreadPool threadPool;
	};
}
