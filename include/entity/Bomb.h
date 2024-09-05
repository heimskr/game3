#pragma once

#include "entity/Projectile.h"

namespace Game3 {
	class Bomb: public Projectile {
		public:
			static Identifier ID() { return {"base", "entity/bomb"}; }

			template <typename... Args>
			static std::shared_ptr<Bomb> create(const std::shared_ptr<Game> &, Args &&...args) {
				return Entity::create<Bomb>(std::forward<Args>(args)...);
			}

			std::string getName() const override { return "Bomb"; }

			void onHit(const EntityPtr &target) final;
			void onExpire() final;

		private:
			Bomb(Identifier item_id = "base:item/bomb", const Vector3 &initial_velocity = {}, double angular_velocity = 0, double linger_time = 0);

		friend class Entity;
	};
}
