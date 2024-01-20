#include "game/HasTickQueue.h"

namespace Game3 {
	void HasTickQueue::tick() {
		++currentTick;
		dequeueAll();
	}

	void HasTickQueue::tick(double delta, double frequency) {
		currentTick += Tick(std::max(1.0, delta * frequency));
		dequeueAll();
	}

	void HasTickQueue::enqueueAt(Function function, Tick moment) {
		auto lock = tickQueue.uniqueLock();
		tickQueue.emplace(moment, std::move(function));
	}

	void HasTickQueue::dequeueAll() {
		auto lock = tickQueue.uniqueLock();

		// Call and remove all queued functions that should execute now or should've been executed by now.
		for (auto iter = tickQueue.begin(); iter != tickQueue.end() && iter->first <= currentTick;) {
			iter->second(currentTick.load());
			iter = tickQueue.erase(iter);
		}
	}
}
