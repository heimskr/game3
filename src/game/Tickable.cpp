#include "game/Game.h"
#include "game/Tickable.h"

namespace Game3 {
	void Tickable::tryEnqueueTick() {
		Game &game = getGame();

		const Tick next_tick = game.getCurrentTick() + 1;

		if (tickSet.contains(next_tick))
			return;

		tickSet.insert(next_tick);
		enqueueTick();
	}

	void Tickable::didTick() {
		tickSet.erase(getGame().getCurrentTick());
	}

	Tick Tickable::tickEnqueued(Tick tick) {
		tickSet.insert(tick);
		return tick;
	}
}
