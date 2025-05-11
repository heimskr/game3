#pragma once

#include "entity/Entity.h"
#include "graphics/Color.h"
#include "types/Encodable.h"

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

			void renderShadow(const RendererContext &) override;
			void render(const RendererContext &) override;
			void tick(const TickArgs &) override;
			bool shouldPersist() const override { return false; }
			void onSpawn() override;
			void setRandomizationParameters(Buffer) override;
			std::string getName() const override { return "Square Particle"; }

			int getZIndex() const override;

			void encode(Buffer &) override;
			void decode(Buffer &) override;

			struct RandomizationOptions {
				float sizeMin = 0.5;
				float sizeMax = 0.5;
				float hueMin{};
				float hueMax{};
				float saturationMin{};
				float saturationMax{};
				float valueMin{};
				float valueMax{};
				float alphaMin = 1.0;
				float alphaMax = 1.0;

				void encode(Buffer &);
				void decode(Buffer &);
			};

		private:
			Vector3 initialVelocity;
			float size = 16;
			Color color;
			double depth = 0;
			double lingerTime = DEFAULT_LINGER_TIME;
			std::optional<RandomizationOptions> randomizationOptions;

			SquareParticle(const Vector3 &initial_velocity = {}, float size = 0.5, Color = {1, 1, 1, 1}, double depth = 0, double linger_time = DEFAULT_LINGER_TIME);

		friend class Entity;
	};
}
