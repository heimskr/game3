#pragma once

#include "statuseffect/TimedStatusEffect.h"

namespace Game3 {
	class Corroded: public TimedStatusEffect {
		public:
			static Identifier ID() { return {"base", "statuseffect/corroded"}; }

			float severity{};

			Corroded();
			Corroded(float duration, float severity);

			std::string getName() const final;
			bool apply(const std::shared_ptr<LivingEntity> &, float delta) final;
			void modifyColors(Color &multiplier, Color &composite) final;
			void encode(Buffer &) final;
			void decode(Buffer &) final;
			std::unique_ptr<StatusEffect> copy() const final;

		private:
			float accumulatedDamage{};
	};
}
