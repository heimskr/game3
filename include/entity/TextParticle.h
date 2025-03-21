#pragma once

#include "entity/Entity.h"
#include "graphics/TextRenderer.h"
#include "types/UString.h"

namespace Game3 {
	class TextParticle: public Entity {
		private:
			static constexpr double DEFAULT_LINGER_TIME = 1;
			static constexpr TextAlign DEFAULT_ALIGN = TextAlign::Center;

		public:
			static Identifier ID() { return {"base", "entity/text_particle"}; }

			template <typename... Args>
			static std::shared_ptr<TextParticle> create(const std::shared_ptr<Game> &, Args &&...args) {
				return Entity::create<TextParticle>(std::forward<Args>(args)...);
			}

			void renderShadow(const RendererContext &) override;
			void render(const RendererContext &) override;
			void tick(const TickArgs &) override;
			bool shouldPersist() const override { return false; }
			void onSpawn() override;
			std::string getName() const override { return "Text Particle"; }

			int getZIndex() const override;

			void encode(Buffer &) override;
			void decode(Buffer &) override;

		private:
			UString text;
			Color color;
			double lingerTime = DEFAULT_LINGER_TIME;
			TextAlign align = DEFAULT_ALIGN;
			double age = 0;

			TextParticle();
			TextParticle(UString, Color = {1, 1, 1, 1}, double linger_time = DEFAULT_LINGER_TIME, TextAlign = DEFAULT_ALIGN);

		friend class Entity;
	};
}
