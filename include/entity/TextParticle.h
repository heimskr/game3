#pragma once

#include "entity/Entity.h"
#include "graphics/TextRenderer.h"

namespace Game3 {
	class TextParticle: public Entity {
		private:
			static constexpr float DEFAULT_LINGER_TIME = 1;
			static constexpr TextAlign DEFAULT_ALIGN = TextAlign::Center;

		public:
			static Identifier ID() { return {"base", "entity/text_particle"}; }

			static std::shared_ptr<TextParticle> create(Game &);
			static std::shared_ptr<TextParticle> create(Game &, Glib::ustring text, Color = {1, 1, 1, 1}, float linger_time = DEFAULT_LINGER_TIME, TextAlign = DEFAULT_ALIGN);

			void render(const RendererSet &) override;
			void tick(Game &, float delta) override;
			bool shouldPersist() const override { return false; }
			void onSpawn() override;
			std::string getName() const override { return "Text Particle"; }

			int getZIndex() const override;

			void encode(Buffer &) override;
			void decode(Buffer &) override;

		private:
			Glib::ustring text;
			Color color;
			float lingerTime = DEFAULT_LINGER_TIME;
			TextAlign align = DEFAULT_ALIGN;
			float age = 0;

			TextParticle();
			TextParticle(Glib::ustring, Color = {1, 1, 1, 1}, float linger_time = DEFAULT_LINGER_TIME, TextAlign = DEFAULT_ALIGN);

		friend class Entity;
	};
}
