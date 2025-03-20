#pragma once

#include "entity/Projectile.h"
#include "graphics/Color.h"

namespace Game3 {
	class FluidParticle: public Projectile {
		private:
			static constexpr double DEFAULT_LINGER_TIME = 1;

		public:
			static Identifier ID() { return {"base", "entity/fluid_particle"}; }

			template <typename... Args>
			static std::shared_ptr<FluidParticle> create(const std::shared_ptr<Game> &, Args &&...args) {
				return Entity::create<FluidParticle>(std::forward<Args>(args)...);
			}

			void render(const RendererContext &) final;
			bool shouldPersist() const final { return false; }
			std::string getName() const final { return "Fluid Particle"; }

			std::shared_ptr<Texture> getTexture() override;
			void onHit(const EntityPtr &target) final;
			int getZIndex() const final;

			void encode(Buffer &) final;
			void decode(Buffer &) final;

		private:
			double depth = 0;
			float size = 16;
			Color color;
			FluidID fluidID{};

			FluidParticle(FluidID fluidID = -1, const Vector3 &initialVelocity = {}, float size = 16, Color = {1, 1, 1, 1}, double depth = 0, double lingerTime = DEFAULT_LINGER_TIME);

		friend class Entity;
	};
}
