#pragma once

#include "entity/Projectile.h"

namespace Game3 {
	class Snowball: public Projectile {
		public:
			static Identifier ID() { return {"base", "entity/snowball"}; }

			template <typename... Args>
			static std::shared_ptr<Snowball> create(const std::shared_ptr<Game> &, Args &&...args) {
				return Entity::create<Snowball>(std::forward<Args>(args)...);
			}

			std::string getName() const override { return "Snowball"; }

			void onHit(const EntityPtr &target) final;

		private:
			Snowball(Identifier item_id = "base:item/snowball", const Vector3 &initial_velocity = {}, double angular_velocity = 0, double linger_time = 5);

		friend class Entity;
	};
}
