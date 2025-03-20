#pragma once

#include "registry/Registerable.h"

#include <memory>

namespace Game3 {
	struct Color;
	class LivingEntity;

	// Living entities hold separate instances of `StatusEffect` subclasses.
	// They're not meant to be singletons as e.g. Tiles and Items are.
	class StatusEffect: public NamedRegisterable {
		public:
			virtual ~StatusEffect();

			/** Returns whether the effect should be cleared. */
			virtual bool apply(const std::shared_ptr<LivingEntity> &, float delta) = 0;

			virtual void onRemove(const std::shared_ptr<LivingEntity> &);

			virtual void modifyColor(Color &);

		protected:
			StatusEffect(Identifier identifier);
	};
}
