#include "game/Game.h"
#include "game/Tickable.h"

namespace Game3 {
	void Tickable::tryEnqueueTick() {
		didTick();

		const Tick next_tick = getGame().getCurrentTick() + 1;

		if (tickSet.contains(next_tick))
			return;

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
