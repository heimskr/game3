#include "entity/Chicken.h"
#include "entity/Egg.h"
#include "entity/Quarter.h"
#include "game/Game.h"
#include "realm/Realm.h"
#include "threading/ThreadContext.h"

namespace Game3 {
	Egg::Egg(Identifier item_id, const Vector3 &initial_velocity, double angular_velocity, double linger_time):
		Projectile(ID(), std::move(item_id), initial_velocity, angular_velocity, linger_time) {}

	void Egg::onHit(const EntityPtr &target) {
		if (getSide() != Side::Server) {
			return;
		}

		hasHit = true;
		applyKnockback(target, 1.5);
	}

	void Egg::onExpire() {
		RealmPtr realm = getRealm();
		GamePtr game = realm->getGame();

		queueDestruction();

		if (game->getSide() != Side::Server) {
			return;
		}

		const float random = threadContext.random(0.f, 1.f);

		int count{};

		// You see, the joke is (spoiler alert):
		// 1/128 chance of four chickens
		// 1/32 chance of at least 2 chickens
		// 1/4 chance of at least 1 chicken
		// 1/1 chance of at least a quarter of a chicken.

		if (random < 1 / 128.f) {
			count = 4;
		} else if (random < 1 / 32.f) {
			count = 2;
		} else if (random < 1 / 4.f) {
			count = 1;
		} else {
			EntityPtr quarter = Quarter::create(game);
			quarter->spawning = true;
			quarter->offset = offset.copyBase();

			double intpart{};

			Position new_pos = getPosition();
			quarter->offset.x = std::modf(quarter->offset.x, &intpart);
			new_pos.column += intpart;
			quarter->offset.y = std::modf(quarter->offset.y, &intpart);
			new_pos.row += intpart;
			quarter->setRealm(realm);
			realm->queueEntityInit(std::move(quarter), new_pos);
			realm->playSound(new_pos, "base:sound/flesh", threadContext.getPitch(1.25f));
			return;
		}

		for (int i = 0; i < count; ++i) {
			EntityPtr chicken = Chicken::create(game);
			chicken->spawning = true;
			chicken->offset = offset.copyBase();
			chicken->setRealm(realm);
			chicken->velocity = Vector3{threadContext.random(-2.0, 2.0), 0.0, 2.0};
			realm->queueEntityInit(std::move(chicken), getPosition());
		}
	}
}
