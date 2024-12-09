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
			constexpr static std::size_t PATHFIND_MAX = 256;

			static std::uniform_real_distribution<float> getWanderDistribution();

			Index wanderRadius = 8;

			void toJSON(boost::json::value &) const override;
			void absorbJSON(const std::shared_ptr<Game> &, const boost::json::value &) override;
			void updateRiderOffset(const std::shared_ptr<Entity> &rider) override;
			bool onInteractOn(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			bool onInteractNextTo(const std::shared_ptr<Player> &, Modifiers, const ItemStackPtr &, Hand) override;
			void init(const std::shared_ptr<Game> &) override;
			void tick(const TickArgs &) override;
			float getMovementSpeed() const override { return 5.f; }
			HitPoints getMaxHealth() const override;
			virtual bool wander();
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
