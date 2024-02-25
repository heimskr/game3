#include "game/Game.h"
#include "game/Tickable.h"

namespace Game3 {
	bool Tickable::tryInitialTick() {
		return !initialTickDone.exchange(true);
	}

	void Tickable::tryEnqueueTick() {
		didTick();

		GamePtr game = getGame();
		const Tick next_tick = game->getCurrentTick() + 1;

		if (tickSet.contains(next_tick))
			return;

		enqueueTick();
	}

	void Tickable::didTick() {
		GamePtr game = getGame();
		tickSet.erase(game->getCurrentTick());
	}

	Tick Tickable::tickEnqueued(Tick tick) {
		tickSet.insert(tick);
		return tick;
	}
}
