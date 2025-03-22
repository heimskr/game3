#pragma once

#include "statuseffect/TexturedStatusEffect.h"

namespace Game3 {
	class Burning: public TexturedStatusEffect {
		public:
			static Identifier ID() { return {"base", "statuseffect/burning"}; }

			float duration{};
			float severity{};

			Burning();
			Burning(float duration, float severity);

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
