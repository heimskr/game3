#include "entity/Snowball.h"

namespace Game3 {
	Snowball::Snowball(Identifier itemID, const Vector3 &initialVelocity, double angularVelocity, const std::optional<Position> &intendedTarget, double lingerTime):
		Projectile(ID(), std::move(itemID), initialVelocity, angularVelocity, intendedTarget, lingerTime) {}

	void Snowball::onHit(const EntityPtr &target) {
		if (getSide() != Side::Server) {
			return;
		}

		hasHit = true;
		applyKnockback(target, 1.5);
	}
}
