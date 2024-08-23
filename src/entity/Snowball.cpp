#include "entity/Snowball.h"
#include "game/Game.h"
#include "realm/Realm.h"

namespace Game3 {
	Snowball::Snowball(Identifier item_id, const Vector3 &initial_velocity, double angular_velocity, double linger_time):
		Projectile(ID(), std::move(item_id), initial_velocity, angular_velocity, linger_time) {}

	void Snowball::onHit(const EntityPtr &target) {
		if (getSide() != Side::Server) {
			return;
		}

		hasHit = true;

		target->velocity.withUnique([this](Vector3 &target_velocity) {
			auto lock = velocity.uniqueLock();
			target_velocity.x += velocity.x;
			target_velocity.y += velocity.y;
			target_velocity.z = std::max(target_velocity.z, std::abs(velocity.z) * 1.5);
			velocity.x = 0;
			velocity.y = 0;
			velocity.z = 0;
		});

		target->offset.withUnique([](Vector3 &offset) {
			offset.z += 1;
		});

		target->increaseUpdateCounter();
		target->sendToVisible();
		increaseUpdateCounter();
		sendToVisible();
	}
}
