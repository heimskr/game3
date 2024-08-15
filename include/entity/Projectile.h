#pragma once

#include "entity/Entity.h"
#include "graphics/TextRenderer.h"

namespace Game3 {
	class Projectile: public Entity {
		private:
			static constexpr double DEFAULT_LINGER_TIME = 2;

		public:
			static Identifier ID() { return {"base", "entity/projectile"}; }

			template <typename... Args>
			static std::shared_ptr<Projectile> create(const std::shared_ptr<Game> &, Args &&...args) {
				return Entity::create<Projectile>(std::forward<Args>(args)...);
			}

			void tick(const TickArgs &) override;
			void render(const RendererContext &) override;
			void onSpawn() override;
			std::string getName() const override { return "Projectile"; }

			int getZIndex() const override;

			void encode(Buffer &) override;
			void decode(Buffer &) override;

			const Identifier & getItemID() const;

		protected:
			std::shared_ptr<Texture> getTexture() override;

		private:
			Identifier itemID;
			Vector3 initialVelocity;
			double lingerTime{};
			float offsetX = 0;
			float offsetY = 0;
			float sizeX = 16;
			float sizeY = 16;
			bool needsTexture = true;

			Projectile(Identifier item_id = "base:item/snowball", const Vector3 &initial_velocity = {}, double linger_time = DEFAULT_LINGER_TIME);

			void setTexture(const ClientGamePtr &);

		friend class Entity;
	};
}
