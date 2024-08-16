#pragma once

#include "entity/Entity.h"
#include "graphics/TextRenderer.h"

namespace Game3 {
	class SquareParticle: public Entity {
		private:
			static constexpr double DEFAULT_LINGER_TIME = 1;

		public:
			static Identifier ID() { return {"base", "entity/square_particle"}; }

			template <typename... Args>
			static std::shared_ptr<SquareParticle> create(const std::shared_ptr<Game> &, Args &&...args) {
				return Entity::create<SquareParticle>(std::forward<Args>(args)...);
			}

			void render(const RendererContext &) override;
			void tick(const TickArgs &) override;
			bool shouldPersist() const override { return false; }
			void onSpawn() override;
			std::string getName() const override { return "Square Particle"; }

			int getZIndex() const override;

			void encode(Buffer &) override;
			void decode(Buffer &) override;

		private:
			Vector3 initialVelocity;
			float size = 16;
			Color color;
			double depth = 0;
			double lingerTime = DEFAULT_LINGER_TIME;

			SquareParticle(const Vector3 &initial_velocity = {}, float size = 16, Color = {1, 1, 1, 1}, double depth = 0, double linger_time = DEFAULT_LINGER_TIME);

		friend class Entity;
	};
}
