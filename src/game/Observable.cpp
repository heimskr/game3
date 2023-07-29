#include "game/Observable.h"

namespace Game3 {
	void Observable::addObserver(const std::shared_ptr<Player> &player) {
		auto lock = observers.uniqueLock();
		observers.emplace(player);
	}

	void Observable::removeObserver(const std::shared_ptr<Player> &player) {
		auto lock = observers.uniqueLock();
		observers.erase(player);
	}

	void Observable::cleanObservers() {
		auto lock = observers.uniqueLock();
		std::erase_if(observers, [](const std::weak_ptr<Player> &player) {
			return player.expired();
		});
	}
}
