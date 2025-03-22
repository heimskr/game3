#pragma once

#include "entity/Entity.h"
#include "graphics/Color.h"

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
			virtual void onHit(const EntityPtr &target);
			virtual void onExpire();
			void setThrower(const EntityPtr &);

		protected:
			Identifier itemID;
			Vector3 initialVelocity;
			double angularVelocity = 0;
			double lingerTime{};

			float offsetX = 0;
			float offsetY = 0;
			float sizeX = 16;
			float sizeY = 16;
			float scale = .5;
			double angle = 0;
			bool needsTexture = true;
			bool hasHit = false;
			GlobalID thrower = -1;

			Projectile(EntityType type, Identifier item_id, const Vector3 &initial_velocity = {}, double angular_velocity = 0, double linger_time = DEFAULT_LINGER_TIME);

			std::shared_ptr<Texture> getTexture() override;
			void setTexture(const ClientGamePtr &);
			virtual void applyKnockback(const EntityPtr &, float factor);

		friend class Entity;
	};
}
