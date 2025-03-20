#pragma once

#include "statuseffect/StatusEffect.h"

namespace Game3 {
	class Burning: public StatusEffect {
		public:
			static Identifier ID() { return {"base", "statuseffect/burning"}; }

			float duration{};
			float severity{};

			Burning(float duration, float severity);

			bool apply(const std::shared_ptr<LivingEntity> &, float delta) final;

			void modifyColor(Color &) final;

		private:
			float accumulatedDamage{};
	};
}
