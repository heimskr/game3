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
			Bomb(Identifier itemID = "base:item/bomb", const Vector3 &initialVelocity = {}, double angularVelocity = 0, const std::optional<Position> &intendedTarget = {}, double lingerTIme = 0);

		friend class Entity;
	};
}
