#include "entity/Chicken.h"
#include "game/Game.h"

namespace Game3 {
	namespace {
		constexpr std::chrono::seconds EGG_PERIOD{150};
	}

	void Chicken::tick(const TickArgs &args) {
		if (getSide() == Side::Server) {
			if (firstEgg) {
				firstEgg = false;
			} else if (eggTick <= args.game.getCurrentTick()) {
				layEgg();
				eggTick = enqueueTick(EGG_PERIOD);
			}
		}

		Animal::tick(args);
	}

	void Chicken::layEgg() {
		ItemStack(*game, "base:item/egg").spawn(getRealm(), position);
	}
}
