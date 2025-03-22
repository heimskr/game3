#pragma once

#include "statuseffect/TexturedStatusEffect.h"

namespace Game3 {
	class Chilling: public TexturedStatusEffect {
		public:
			static Identifier ID() { return {"base", "statuseffect/chilling"}; }

			float duration{};

			Chilling();
			Chilling(float duration);

			std::string getName() const final;
			bool apply(const std::shared_ptr<LivingEntity> &, float delta) final;
			void onAdd(const std::shared_ptr<LivingEntity> &) final;
			void onRemove(const std::shared_ptr<LivingEntity> &) final;
			void modifyColors(Color &multiplier, Color &composite) final;
			void encode(Buffer &) final;
			void decode(Buffer &) final;
			std::unique_ptr<StatusEffect> copy() const final;

		private:
			bool wasAdded = false;
	};
}
