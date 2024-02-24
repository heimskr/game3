#pragma once

#include "entity/LivingEntity.h"

// When attacked, monsters will attack their attacker and move towards them if the attacker tries to escape.
// If they go a certain duration of time without landing a hit, they give up chasing the attacker.

namespace Game3 {
	class Monster: public LivingEntity {
		public:
			void tick(const TickArgs &) override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			HitPoints getMaxHealth() const override;
			void onAttack(const std::shared_ptr<LivingEntity> &) override;

		protected:
			// Synchronized
			GlobalID targetGID = -1;

			// Not synchronized
			double timeSinceAttack = 0;
			double timeSinceSearch = 0;
			double timeSinceAdjustment = 0;
			std::weak_ptr<LivingEntity> weakTarget;

			Monster();

			virtual HitPoints getBaseDamage() const = 0;
			virtual int getVariability() const = 0;
			virtual double getAttackPeriod() const = 0;
			virtual double getPatience() const;
			virtual uint64_t getSearchRadius() const;
			virtual uint64_t getTenacity() const;
			virtual bool canDespawn() const;
			virtual double getMinimumAgeForDespawn() const;

			bool isSpawnableMonster() const override { return true; }
			std::vector<ItemStack> getDrops() override;

			std::shared_ptr<LivingEntity> getTarget();
			void setTarget(const std::shared_ptr<LivingEntity> &);
			bool hasTarget();
			void giveUp();
			bool isNearTarget();
			bool isFacingTarget();
			void faceTarget();
			void followTarget();
			bool tryAttack();
			void attack(const std::shared_ptr<LivingEntity> &);
			void search();
	};
}
