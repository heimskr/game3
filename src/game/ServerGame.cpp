#include "game/ServerGame.h"

namespace Game3 {
	void ServerGame::tick() {
		auto now = getTime();
		auto difference = now - lastTime;
		lastTime = now;
		delta = std::chrono::duration_cast<std::chrono::nanoseconds>(difference).count() / 1'000'000'000.;
		for (auto &[id, realm]: realms)
			realm->tick(delta);
		for (const auto &player: players)
			player->ticked = false;
	}
}
