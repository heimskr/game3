#include "entity/Chicken.h"
#include "game/Game.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::seconds EGG_PERIOD{150};
	}

	void Chicken::tick(Game &game, float delta) {
		if (getSide() == Side::Server) {
			if (firstEgg) {
				firstEgg = false;
			} else if (eggTick <= game.getCurrentTick()) {
				layEgg();
				eggTick = tickEnqueued(enqueueTick(EGG_PERIOD));
			}
		}

		Animal::tick(game, delta);
	}

	void Chicken::layEgg() {
		ItemStack(*game, "base:item/egg").spawn(getRealm(), position);
	}
}
