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
			virtual HitPoints getDefense() const { return 0; }

			void onCreate() override;
			void toJSON(nlohmann::json &) const override;
			void absorbJSON(Game &, const nlohmann::json &) override;
			void render(const RendererSet &) override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			virtual bool canShowHealthBar() const;

		protected:
			LivingEntity();
	};
}
