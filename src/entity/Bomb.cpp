#include "entity/Bomb.h"
#include "game/Game.h"
#include "realm/Realm.h"

namespace Game3 {
	Bomb::Bomb(Identifier item_id, const Vector3 &initial_velocity, double angular_velocity, double linger_time):
		Projectile(ID(), std::move(item_id), initial_velocity, angular_velocity, linger_time) {}

	void Bomb::onHit(const EntityPtr &target) {
		if (getSide() != Side::Server)
			return;

		hasHit = true;
		applyKnockback(target, 3);
	}

	void Bomb::onExpire() {
		RealmPtr realm = getRealm();
		GamePtr game = realm->getGame();

		if (game->getSide() != Side::Server)
			return;

		queueDestruction();
		constexpr static Index DIAMETER = 5;
		constexpr static double RADIUS = DIAMETER / 2.;

		auto [prow, pcol] = getPosition();
		prow += offset.y;
		pcol += offset.x;

		for (Index row = prow - DIAMETER; row <= prow + DIAMETER; ++row) {
			for (Index column = pcol - DIAMETER; column <= pcol + DIAMETER; ++column) {
				const Position pos(row, column);
				if (RADIUS < pos.distance({prow, pcol}))
					continue;
				realm->damageGround(pos);
				if (TileEntityPtr tile = realm->tileEntityAt(pos); tile && tile->kill())
					tile->destroy();
			}
		}
	}
}
