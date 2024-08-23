#include "entity/Snowball.h"
#include "game/Game.h"
#include "realm/Realm.h"

namespace Game3 {
	Snowball::Snowball(Identifier item_id, const Vector3 &initial_velocity, double angular_velocity, double linger_time):
		Projectile(ID(), std::move(item_id), initial_velocity, angular_velocity, linger_time) {}

	void Snowball::onHit(const EntityPtr &target) {
		target->velocity.withUnique([this](Vector3 &target_velocity) {
			auto lock = velocity.sharedLock();
			target_velocity += velocity;
		});

		target->increaseUpdateCounter();

		queueDestruction();
	}
}
