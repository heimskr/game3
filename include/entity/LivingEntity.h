#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class LivingEntity: public virtual Entity {
		public:
			constexpr static HitPoints INVINCIBLE = 0;
			Atomic<HitPoints> health = 0;

			/** Returns the maximum number of hitpoints this entity can have. If 0, the entity is invincible. */
			virtual HitPoints getMaxHealth() const { return 0; }
			bool isInvincible() const { return getMaxHealth() == INVINCIBLE; }

			void onSpawn() override;
			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void render(const RendererSet &) override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			virtual bool canShowHealthBar() const;
			virtual int getDefense() const;
			virtual double getLuck() const;
			virtual void takeDamage(HitPoints);
			virtual void kill();

		protected:
			int defenseStat = 0;
			double luckStat = 0;

			LivingEntity();
	};
}
