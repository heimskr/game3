#include "entity/Chicken.h"
#include "entity/ItemEntity.h"
#include "game/Game.h"

namespace {
	constexpr std::chrono::seconds EGG_PERIOD{150};
}

namespace Game3 {
	void Chicken::tick(const TickArgs &args) {
		if (args.game->getSide() == Side::Server) {
			const Tick current_tick = args.game->getCurrentTick();
			if (firstEgg) {
				firstEgg = false;
				eggTick = current_tick + args.game->getDelayTicks(EGG_PERIOD);
			} else if (eggTick <= current_tick) {
				layEgg();
				eggTick = enqueueTick(EGG_PERIOD);
			}
		}

		Animal::tick(args);
	}

	void Chicken::layEgg() {
		RealmPtr realm = getRealm();
		ItemStack::spawn(Place{position, realm}, realm->getGame(), "base:item/egg")->tryEnqueueTick();
	}
}
