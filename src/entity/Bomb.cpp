#include "entity/Bomb.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "util/Explosion.h"

namespace Game3 {
	Bomb::Bomb(Identifier item_id, const Vector3 &initial_velocity, double angular_velocity, double linger_time):
		Projectile(ID(), std::move(item_id), initial_velocity, angular_velocity, linger_time) {}

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

		if (game->getSide() != Side::Server) {
			return;
		}

		queueDestruction();
		constexpr static Index DIAMETER = 5;
		constexpr static double RADIUS = DIAMETER / 2.;

		Position position = getPosition();
		position.row += offset.y;
		position.column += offset.x;

		causeExplosion(Place{position, realm}, RADIUS, 0, true);
	}
}
