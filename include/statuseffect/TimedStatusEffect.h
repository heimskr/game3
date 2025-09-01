#pragma once

#include "statuseffect/TexturedStatusEffect.h"

namespace Game3 {
	class TimedStatusEffect: public TexturedStatusEffect {
		public:
			float duration{};

			TimedStatusEffect(Identifier identifier, Identifier itemID, float duration);

			bool apply(const std::shared_ptr<LivingEntity> &, float delta) override;
			void replenish(const std::shared_ptr<LivingEntity> &) override;
			void encode(Buffer &) override;
			void decode(BasicBuffer &) override;

		private:
			float originalDuration{};
	};
}
