#pragma once

#include "statuseffect/TimedStatusEffect.h"

namespace Game3 {
	class Chilling: public TimedStatusEffect {
		public:
			static Identifier ID() { return {"base", "statuseffect/chilling"}; }

			Chilling();
			Chilling(float duration);

			std::string getName() const final;
			void onAdd(const std::shared_ptr<LivingEntity> &) final;
			void onRemove(const std::shared_ptr<LivingEntity> &) final;
			void modifyColors(Color &multiplier, Color &composite) final;
			std::unique_ptr<StatusEffect> copy() const final;
	};
}
