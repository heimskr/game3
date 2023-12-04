#pragma once

#include "entity/LivingEntity.h"

// When attacked, monsters will attack their attacker and move towards them if the attacker tries to escape.
// If they go a certain duration of time without landing a hit, they give up chasing the attacker.

namespace Game3 {
	class Monster: public LivingEntity {
		public:
			void tick(Game &, float) override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			HitPoints getMaxHealth() const override;
			void onAttack(const std::shared_ptr<LivingEntity> &) override;

		protected:
			float timeSinceAttack = 0;
			GlobalID targetGID = -1;
			std::weak_ptr<LivingEntity> weakTarget;

			Monster();

			virtual HitPoints getBaseDamage() const = 0;
			virtual int getVariability() const = 0;
			virtual float getAttackPeriod() const = 0;
			virtual float getPatience() const;

			std::shared_ptr<LivingEntity> getTarget();
			bool isAttacking();
			void giveUp();
			bool isNearTarget();
			bool isFacingTarget();
			void faceTarget();
			void followTarget();
			bool tryAttack();
			void attack(const std::shared_ptr<LivingEntity> &);
	};
}
