#pragma once

#include "entity/Entity.h"

namespace Game3 {
	class ExplosionParticle: public Entity {
		public:
			static Identifier ID() { return {"base", "entity/explosion_particle"}; }

			static auto create(const std::shared_ptr<Game> &) {
				return Entity::create<ExplosionParticle>();
			}

			std::string getName() const override { return "Explosion"; }

			void tick(const TickArgs &) override;
			void render(const RendererContext &) override;
			void renderShadow(const RendererContext &) override;
			bool isVisible() const override;
			bool visibilityMatters() const override;
			void encode(Buffer &) override;
			void decode(Buffer &) override;

			static float getLifetime();

		protected:
			ExplosionParticle();

		friend class Entity;
	};
}
