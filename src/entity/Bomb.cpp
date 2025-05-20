#include "entity/Bomb.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "util/Explosion.h"

namespace Game3 {
	Bomb::Bomb(Identifier itemID, const Vector3 &initialVelocity, double angularVelocity, const std::optional<Position> &intendedTarget, double lingerTime):
		Projectile(ID(), std::move(itemID), initialVelocity, angularVelocity, intendedTarget, lingerTime) {}

	void Bomb::onHit(const EntityPtr &target) {
		if (getSide() != Side::Server) {
			return;
		}

		hasHit = true;
		applyKnockback(target, 3);
	}

	void Bomb::onExpire() {
		RealmPtr realm = getRealm();
		GamePtr game = realm->getGame();

		queueDestruction();

		if (game->getSide() != Side::Server) {
			return;
		}

		constexpr Index DIAMETER = 5;
		constexpr float RADIUS = DIAMETER / 2.;

		Position position = getPosition();
		position.row = std::floor(position.row + offset.y);
		position.column = std::floor(position.column + offset.x);

		if (intendedTarget && position.taxiDistance(*intendedTarget) <= 2) {
			causeExplosion(Place{*intendedTarget, realm}, RADIUS, true);
		} else {
			causeExplosion(Place{position, realm}, RADIUS, true);
		}
	}
}
