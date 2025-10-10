#pragma once

#include "game/HasGame.h"
#include "types/Types.h"

namespace Game3 {
	class Game;

	struct TickArgs: HasGame {
		Tick tick;
		float delta;

		TickArgs(const GamePtr &game, Tick tick, float delta):
			HasGame(game),
			tick(tick),
			delta(delta) {}
	};
}
