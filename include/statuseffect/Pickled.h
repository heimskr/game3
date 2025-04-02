#pragma once

#include "statuseffect/TimedStatusEffect.h"

namespace Game3 {
	class Pickled: public TimedStatusEffect {
		public:
			static Identifier ID() { return {"base", "statuseffect/pickled"}; }

			Pickled();
			Pickled(float duration);

			std::string getName() const final;
			void modifyColors(Color &multiplier, Color &composite) final;
			std::unique_ptr<StatusEffect> copy() const final;
	};
}
