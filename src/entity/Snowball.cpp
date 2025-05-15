#include "entity/Snowball.h"

namespace Game3 {
	Snowball::Snowball(Identifier item_id, const Vector3 &initial_velocity, double angular_velocity, double linger_time):
		Projectile(ID(), std::move(item_id), initial_velocity, angular_velocity, linger_time) {}

	void Snowball::onHit(const EntityPtr &target) {
		if (getSide() != Side::Server) {
			return;
		}

		hasHit = true;
		applyKnockback(target, 1.5);
	}
}
