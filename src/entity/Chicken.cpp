#include "entity/Chicken.h"
#include "game/Game.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::seconds EGG_PERIOD{150};
	}

	void Chicken::tick(Game &game, float delta) {
		Animal::tick(game, delta);

		if (getSide() != Side::Server)
			return;

		if (firstEgg) {
			firstEgg = false;
		} else if (eggTick <= game.getCurrentTick()) {
			layEgg();
		}

		eggTick = enqueueTick(EGG_PERIOD);
	}

	void Chicken::layEgg() {
		ItemStack(*game, "base:item/egg").spawn(getRealm(), position);
	}
}
