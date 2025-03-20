#pragma once

#include "statuseffect/StatusEffect.h"

namespace Game3 {
	class Burning: public StatusEffect {
		public:
			float duration{};
			float severity{};

			Burning(float duration, float severity);

			bool apply(const std::shared_ptr<LivingEntity> &, float delta) final;

		private:
			float accumulatedDamage{};
	};
}
