#pragma once

#include "entity/Projectile.h"

namespace Game3 {
	class Egg: public Projectile {
		public:
			static Identifier ID() { return {"base", "entity/egg"}; }

			template <typename... Args>
			static std::shared_ptr<Egg> create(const std::shared_ptr<Game> &, Args &&...args) {
				return Entity::create<Egg>(std::forward<Args>(args)...);
			}

			std::string getName() const override { return "Egg"; }

			void onHit(const EntityPtr &target) final;
			void onExpire() final;

		private:
			Egg(Identifier item_id = "base:item/egg", const Vector3 &initial_velocity = {}, double angular_velocity = 0, double linger_time = 0);

		friend class Entity;
	};
}