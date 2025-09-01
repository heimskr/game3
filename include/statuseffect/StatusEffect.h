#pragma once

#include "registry/Registerable.h"

#include <memory>
#include <string>

namespace Game3 {
	class BasicBuffer;
	class Buffer;
	class ClientGame;
	class LivingEntity;
	class Texture;
	struct Color;

	// Living entities hold separate instances of `StatusEffect` subclasses.
	// They're not meant to be singletons as e.g. Tiles and Items are.
	class StatusEffect: public NamedRegisterable {
		public:
			virtual ~StatusEffect();

			virtual std::string getName() const = 0;

			/** Returns whether the effect should be cleared. */
			virtual bool apply(const std::shared_ptr<LivingEntity> &, float delta) = 0;

			virtual void replenish(const std::shared_ptr<LivingEntity> &);

			virtual void onAdd(const std::shared_ptr<LivingEntity> &);

			virtual void onRemove(const std::shared_ptr<LivingEntity> &);

			virtual void modifyColors(Color &multiplier, Color &composite);

			virtual void encode(Buffer &) = 0;

			virtual void decode(BasicBuffer &) = 0;

			virtual std::unique_ptr<StatusEffect> copy() const = 0;

			virtual std::shared_ptr<Texture> getTexture(const std::shared_ptr<ClientGame> &);

		protected:
			StatusEffect(Identifier identifier);
	};
}
