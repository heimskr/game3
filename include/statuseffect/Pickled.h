#pragma once

#include "statuseffect/StatusEffect.h"

namespace Game3 {
	class Pickled: public StatusEffect {
		public:
			static Identifier ID() { return {"base", "statuseffect/pickled"}; }

			float duration{};

			Pickled();
			Pickled(float duration);

			std::string getName() const final;
			bool apply(const std::shared_ptr<LivingEntity> &, float delta) final;
			void modifyColors(Color &multiplier, Color &composite) final;
			void encode(Buffer &) final;
			void decode(Buffer &) final;
			std::unique_ptr<StatusEffect> copy() const final;
			std::shared_ptr<Texture> getTexture(const std::shared_ptr<ClientGame> &) final;

		private:
			std::shared_ptr<Texture> cachedTexture;
	};
}
