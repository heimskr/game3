#pragma once

#include "types/Types.h"

namespace Game3 {
	class Game;

	struct TickArgs {
		Game &game;
		Tick tick;
		double delta;

		TickArgs(Game &game_, Tick tick_, double delta_):
			game(game_), tick(tick_), delta(delta_) {}
	};
}
