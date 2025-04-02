#pragma once

#include "statuseffect/StatusEffect.h"

namespace Game3 {
	class TexturedStatusEffect: public StatusEffect {
		public:
			TexturedStatusEffect(Identifier identifier, Identifier itemID);

			std::shared_ptr<Texture> getTexture(const std::shared_ptr<ClientGame> &) override;

		private:
			Identifier itemID;
			std::shared_ptr<Texture> cachedTexture;
	};
}
